#pragma once

#include <Arduino.h>
#include <MQTT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

// Telemetry-only bench transport. No command subscription or physical output.
class PrivateMqttTelemetry {
public:
    PrivateMqttTelemetry(const char* deviceId, const char* version, const char* mode)
        : deviceId_(deviceId), version_(version), mode_(mode), mqtt_(512), http_(80) {}

    void begin() {
        bootId_ = esp_random();
        Serial.begin(115200);
        Serial.printf("CV_MQTT_READY deviceId=%s outputs=DISABLED\n", deviceId_);
        loadConfig();
        loadApPassword();
        WiFi.persistent(false);
        WiFi.mode(WIFI_AP_STA);
        mqtt_.setOptions(15, true, 1000);
        mqtt_.begin(net_);
        if (configured_) mqtt_.setHost(host_, 1883);
        if (configured_) startWifi();
        startAp();
    }

    void tick(uint32_t now, const String& measurements, const String& actuators,
              const char* origin, bool valid) {
        readSerial();
        if (apActive_) http_.handleClient();
        if (pendingWifi_ && uint32_t(now - pendingAt_) >= 1000u) applyPendingWifi();
        if (!configured_) return;
        if (WiFi.status() != WL_CONNECTED) {
            ntpStarted_ = false;
            if (uint32_t(now - lastWifiAttempt_) >= 10000u) startWifi();
            return;
        }
        if (!ntpStarted_) { configTime(0, 0, "pool.ntp.org"); ntpStarted_ = true; }
        if (!mqtt_.connected()) {
            if (uint32_t(now - lastMqttAttempt_) < 5000u) return;
            lastMqttAttempt_ = now;
            String will = statusPayload(false, now);
            mqtt_.setWill(statusTopic().c_str(), will.c_str(), true, 1);
            if (!mqtt_.connect(deviceId_, user_, mqttPassword_)) {
                Serial.printf("CV_MQTT_DISCONNECTED error=%d returnCode=%d\n",
                    int(mqtt_.lastError()), int(mqtt_.returnCode()));
                return;
            }
            Serial.println("CV_MQTT_CONNECTED");
            mqtt_.publish(statusTopic(), statusPayload(true, now), true, 1);
            lastStatusAt_ = now;
            lastTelemetryAt_ = now - 15000u;
        }
        mqtt_.loop();
        if (uint32_t(now - lastStatusAt_) >= 20000u) {
            if (mqtt_.publish(statusTopic(), statusPayload(true, now), true, 1))
                lastStatusAt_ = now;
        }
        if (uint32_t(now - lastTelemetryAt_) >= 15000u) {
            String payload = telemetryPayload(now, measurements, actuators, origin, valid);
            if (mqtt_.publish(telemetryTopic(), payload, false, 1)) {
                lastTelemetryAt_ = now;
                Serial.printf("CV_TELEMETRY_SENT deviceId=%s\n", deviceId_);
            }
        }
    }

private:
    const char* deviceId_;
    const char* version_;
    const char* mode_;
    WiFiClient net_;
    MQTTClient mqtt_;
    WebServer http_;
    char ssid_[33] = {};
    char wifiPassword_[64] = {};
    char host_[48] = {};
    char user_[33] = {};
    char mqttPassword_[65] = {};
    bool configured_ = false;
    bool ntpStarted_ = false;
    uint32_t bootId_ = 0;
    uint32_t sequence_ = 0;
    uint32_t lastWifiAttempt_ = 0;
    uint32_t lastMqttAttempt_ = 0;
    uint32_t lastStatusAt_ = 0;
    uint32_t lastTelemetryAt_ = 0;
    uint32_t pendingAt_ = 0;
    bool apActive_ = false;
    bool pendingWifi_ = false;
    char apPassword_[17] = {};
    String pendingSsid_;
    String pendingPassword_;
    String pendingHost_;
    String serialLine_;
    String fields_[5];
    int fieldCount_ = -1;
    bool rotation_ = false;

    String statusTopic() const { return String("cidade-viva/v1/") + deviceId_ + "/status"; }
    String telemetryTopic() const { return String("cidade-viva/v1/") + deviceId_ + "/telemetry"; }

    static bool copyField(char* target, size_t capacity, const String& source) {
        if (source.length() == 0 || source.length() >= capacity) return false;
        source.toCharArray(target, capacity);
        return true;
    }

    static bool privateIpv4(const char* host) {
        IPAddress ip;
        if (!ip.fromString(host)) return false;
        return ip[0] == 10 || (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31)
            || (ip[0] == 192 && ip[1] == 168);
    }

    void loadConfig() {
        Preferences prefs;
        String ssid, wifiPassword, host, user, mqttPassword;
        if (prefs.begin("cv-network", true)) {
            ssid = prefs.getString("ssid", "");
            wifiPassword = prefs.getString("wifiPass", "");
            host = prefs.getString("host", "");
            user = prefs.getString("user", "");
            mqttPassword = prefs.getString("mqttPass", "");
            prefs.end();
        }
        if (ssid.isEmpty()) {
            Preferences legacy;
            if (legacy.begin("cv-wifi", true)) {
                String oldSsid = legacy.getString("ssid", "");
                String oldPassword = legacy.getString("pass", "");
                legacy.end();
                if (oldSsid.length() >= 1 && oldSsid.length() <= 32
                    && (oldPassword.isEmpty() || (oldPassword.length() >= 8 && oldPassword.length() <= 63))) {
                    ssid = oldSsid;
                    wifiPassword = oldPassword;
                    if (prefs.begin("cv-network", false)) {
                        prefs.putString("ssid", ssid);
                        prefs.putString("wifiPass", wifiPassword);
                        prefs.end();
                    }
                    Serial.println("CV_WIFI_MIGRATED");
                }
            }
        }
        configured_ = copyField(ssid_, sizeof ssid_, ssid)
            && wifiPassword.length() < sizeof wifiPassword_
            && copyField(host_, sizeof host_, host)
            && privateIpv4(host_)
            && copyField(user_, sizeof user_, user)
            && copyField(mqttPassword_, sizeof mqttPassword_, mqttPassword);
        if (configured_) wifiPassword.toCharArray(wifiPassword_, sizeof wifiPassword_);
        Serial.println(configured_ ? "CV_MQTT_CONFIGURED" : "CV_MQTT_WAITING_USB_CONFIG");
    }

    void startWifi() {
        lastWifiAttempt_ = millis();
        WiFi.begin(ssid_, wifiPassword_);
        Serial.println("CV_WIFI_CONNECTING");
    }

    void loadApPassword() {
        Preferences prefs;
        if (!prefs.begin("cv-network", false)) return;
        String value = prefs.getString("apPass", "");
        if (value.length() != 16) {
            char generated[17];
            snprintf(generated, sizeof generated, "%08lx%08lx",
                     (unsigned long)esp_random(), (unsigned long)esp_random());
            value = generated;
            prefs.putString("apPass", value);
        }
        prefs.end();
        value.toCharArray(apPassword_, sizeof apPassword_);
        Serial.printf("CV_AP_PASSWORD=%s\n", apPassword_);
    }

    void startAp() {
        if (apActive_ || !configured_ || strlen(apPassword_) < 8) return;
        WiFi.mode(WIFI_AP_STA);
        String apName = String("CidadeViva-") + (String(deviceId_) == "esp32-water" ? "Water" : "Energy");
        if (!WiFi.softAP(apName.c_str(), apPassword_)) return;
        http_.on("/", HTTP_GET, [this]() {
            if (http_.client().localIP() != WiFi.softAPIP()) {
                http_.send(403, "text/plain", "Use a rede de configuracao da placa.");
                return;
            }
            http_.send(200, "text/html; charset=utf-8",
                "<!doctype html><html lang='pt-br'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>Cidade Viva - Wi-Fi</title><body><h1>Configurar Wi-Fi da placa</h1>"
                "<p>Informe a rede 2,4 GHz e o IPv4 privado do notebook que executa a ponte MQTT.</p>"
                "<form method='post' action='/configure'>"
                "<label>Rede (SSID) <input name='ssid' maxlength='32' required></label><br>"
                "<label>Senha Wi-Fi <input name='password' type='password' maxlength='63'></label><br>"
                "<label>IP da ponte <input name='host' inputmode='decimal' required></label><br>"
                "<button type='submit'>Salvar e conectar</button></form></body></html>");
        });
        http_.on("/configure", HTTP_POST, [this]() {
            if (http_.client().localIP() != WiFi.softAPIP()) {
                http_.send(403, "text/plain", "Use a rede de configuracao da placa.");
                return;
            }
            String ssid = http_.arg("ssid");
            String password = http_.arg("password");
            String host = http_.arg("host");
            if (ssid.length() < 1 || ssid.length() > 32
                || (password.length() != 0 && (password.length() < 8 || password.length() > 63))
                || host.length() < 7 || host.length() >= sizeof host_
                || !privateIpv4(host.c_str())) {
                http_.send(400, "text/plain; charset=utf-8", "Dados invalidos. Verifique SSID, senha e IP privado.");
                return;
            }
            pendingSsid_ = ssid;
            pendingPassword_ = password;
            pendingHost_ = host;
            pendingAt_ = millis();
            pendingWifi_ = true;
            http_.send(200, "text/html; charset=utf-8",
                "<h1>Configuracao recebida</h1><p>A placa tentara conectar. Volte ao painel Cidade Viva para verificar o status.</p>");
        });
        http_.begin();
        apActive_ = true;
        Serial.printf("CV_AP_READY ssid=%s ip=%s\n", apName.c_str(), WiFi.softAPIP().toString().c_str());
    }

    void applyPendingWifi() {
        pendingWifi_ = false;
        Preferences prefs;
        if (!prefs.begin("cv-network", false)) return;
        prefs.putString("ssid", pendingSsid_);
        prefs.putString("wifiPass", pendingPassword_);
        prefs.putString("host", pendingHost_);
        prefs.end();
        pendingSsid_ = pendingPassword_ = pendingHost_ = "";
        loadConfig();
        mqtt_.disconnect();
        mqtt_.setHost(host_, 1883);
        WiFi.disconnect();
        startWifi();
    }

    void readSerial() {
        for (int i = 0; i < 96 && Serial.available(); ++i) {
            char c = char(Serial.read());
            if (c == '\r') continue;
            if (c != '\n') {
                if (serialLine_.length() >= 96) { serialLine_ = ""; fieldCount_ = -1; }
                else serialLine_ += c;
                continue;
            }
            String line = serialLine_;
            serialLine_ = "";
            if (line == "CVINFO") {
                Serial.printf("CV_NETWORK mode=%d staConnected=%d apIp=%s apSsid=%s channel=%d\n",
                    int(WiFi.getMode()), WiFi.status() == WL_CONNECTED,
                    WiFi.softAPIP().toString().c_str(), WiFi.softAPSSID().c_str(), WiFi.channel());
                continue;
            }
            if (line == "CVNET1" || line == "CVMQTT1") {
                rotation_ = line == "CVMQTT1";
                fieldCount_ = 0;
                continue;
            }
            if (fieldCount_ < 0) continue;
            fields_[fieldCount_++] = line;
            if (fieldCount_ != (rotation_ ? 3 : 5)) continue;
            fieldCount_ = -1;
            char ssid[33], wifiPass[64], host[48], user[33], mqttPass[65];
            const int offset = rotation_ ? 0 : 2;
            bool valid = (!rotation_ || strlen(ssid_) > 0)
                && (rotation_ || (copyField(ssid, sizeof ssid, fields_[0])
                    && fields_[1].length() < sizeof wifiPass))
                && copyField(host, sizeof host, fields_[offset]) && privateIpv4(host)
                && copyField(user, sizeof user, fields_[offset + 1])
                && copyField(mqttPass, sizeof mqttPass, fields_[offset + 2]);
            if (!valid) { Serial.println("CV_MQTT_CONFIG_INVALID"); continue; }
            Preferences prefs;
            if (!prefs.begin("cv-network", false)) { Serial.println("CV_MQTT_CONFIG_FAILED"); continue; }
            if (!rotation_) {
                prefs.putString("ssid", fields_[0]);
                prefs.putString("wifiPass", fields_[1]);
            }
            prefs.putString("host", fields_[offset]);
            prefs.putString("user", fields_[offset + 1]);
            prefs.putString("mqttPass", fields_[offset + 2]);
            prefs.end();
            for (auto& field : fields_) field = "";
            loadConfig();
            if (configured_) {
                mqtt_.disconnect();
                mqtt_.setHost(host_, 1883);
                if (!rotation_ || WiFi.status() != WL_CONNECTED) startWifi();
            }
        }
    }

    String statusPayload(bool online, uint32_t now) const {
        return String("{\"schemaVersion\":\"1.0\",\"deviceId\":\"") + deviceId_
            + "\",\"online\":" + (online ? "true" : "false")
            + ",\"firmwareVersion\":\"" + version_ + "\",\"uptimeMs\":" + now
            + ",\"lastKnownMode\":\"" + mode_ + "\"}";
    }

    String telemetryPayload(uint32_t now, const String& measurements, const String& actuators,
                            const char* origin, bool valid) {
        time_t utc = time(nullptr);
        bool synced = utc >= 1704067200;
        String id = String(deviceId_) + "-" + String(ESP.getEfuseMac(), HEX) + "-"
            + String(bootId_, HEX) + "-" + String(++sequence_);
        String payload = String("{\"schemaVersion\":\"1.0\",\"messageId\":\"") + id
            + "\",\"deviceId\":\"" + deviceId_ + "\",\"uptimeMs\":" + now;
        if (synced) {
            struct tm tmUtc;
            gmtime_r(&utc, &tmUtc);
            char timestamp[25];
            strftime(timestamp, sizeof timestamp, "%Y-%m-%dT%H:%M:%SZ", &tmUtc);
            payload += String(",\"timestamp\":\"") + timestamp + "\"";
        }
        payload += String(",\"origin\":\"") + origin + "\",\"quality\":\""
            + (!valid ? "INVALID" : synced ? "VALID" : "UNSYNCHRONIZED_TIME")
            + "\",\"mode\":\"" + mode_ + "\",\"measurements\":" + measurements
            + ",\"actuators\":" + actuators + "}";
        return payload;
    }
};
