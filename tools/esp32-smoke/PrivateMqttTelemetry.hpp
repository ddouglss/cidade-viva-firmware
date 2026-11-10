#pragma once

#include <Arduino.h>
#include <MQTT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

// Telemetry-only bench transport. No command subscription or physical output.
class PrivateMqttTelemetry {
public:
    PrivateMqttTelemetry(const char* deviceId, const char* version, const char* mode)
        : deviceId_(deviceId), version_(version), mode_(mode), mqtt_(512) {}

    void begin() {
        bootId_ = esp_random();
        Serial.begin(115200);
        Serial.printf("CV_MQTT_READY deviceId=%s outputs=DISABLED\n", deviceId_);
        loadConfig();
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        mqtt_.setOptions(15, true, 1000);
        mqtt_.begin(net_);
        if (configured_) mqtt_.setHost(host_, 1883);
        if (configured_) startWifi();
    }

    void tick(uint32_t now, const String& measurements, const String& actuators,
              const char* origin, bool valid) {
        readSerial();
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
                Serial.println("CV_MQTT_DISCONNECTED");
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
    String serialLine_;
    String fields_[5];
    int fieldCount_ = -1;

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
        if (!prefs.begin("cv-network", true)) return;
        String ssid = prefs.getString("ssid", "");
        String wifiPassword = prefs.getString("wifiPass", "");
        String host = prefs.getString("host", "");
        String user = prefs.getString("user", "");
        String mqttPassword = prefs.getString("mqttPass", "");
        prefs.end();
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
            if (line == "CVNET1") { fieldCount_ = 0; continue; }
            if (fieldCount_ < 0) continue;
            fields_[fieldCount_++] = line;
            if (fieldCount_ != 5) continue;
            fieldCount_ = -1;
            char ssid[33], wifiPass[64], host[48], user[33], mqttPass[65];
            bool valid = copyField(ssid, sizeof ssid, fields_[0])
                && fields_[1].length() < sizeof wifiPass
                && copyField(host, sizeof host, fields_[2]) && privateIpv4(host)
                && copyField(user, sizeof user, fields_[3])
                && copyField(mqttPass, sizeof mqttPass, fields_[4]);
            if (!valid) { Serial.println("CV_MQTT_CONFIG_INVALID"); continue; }
            Preferences prefs;
            if (!prefs.begin("cv-network", false)) { Serial.println("CV_MQTT_CONFIG_FAILED"); continue; }
            prefs.putString("ssid", fields_[0]);
            prefs.putString("wifiPass", fields_[1]);
            prefs.putString("host", fields_[2]);
            prefs.putString("user", fields_[3]);
            prefs.putString("mqttPass", fields_[4]);
            prefs.end();
            for (auto& field : fields_) field = "";
            loadConfig();
            if (configured_) { mqtt_.setHost(host_, 1883); startWifi(); }
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
