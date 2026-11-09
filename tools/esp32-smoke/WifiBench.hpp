#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <cstring>

namespace {
enum class Phase { Waiting, Connecting, Connected, Finished };

Phase phase = Phase::Waiting;
char ssid[33] = {};
char password[64] = {};
bool readingPassword = false;
bool invalidInput = false;
std::size_t length = 0;
std::uint32_t startedAt = 0;
std::uint32_t lastPromptAt = 0;

bool provisionInitialized = false;
bool apActive = false;
bool saveAfterConnect = false;
String currentDeviceId = "esp32";
String apSsid;
String lastError;
WebServer httpServer(80);
Preferences wifiPrefs;

String jsonEscape(const String& input) {
    String out;
    out.reserve(input.length() + 8);
    for (unsigned i = 0; i < input.length(); ++i) {
        const char c = input[i];
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (static_cast<unsigned char>(c) >= 0x20) {
            out += c;
        }
    }
    return out;
}

const char* phaseLabel() {
    switch (phase) {
    case Phase::Waiting: return "WAITING";
    case Phase::Connecting: return "CONNECTING";
    case Phase::Connected: return "CONNECTED";
    case Phase::Finished: return "FINISHED";
    }
    return "UNKNOWN";
}

void sendCorsHeaders() {
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void resetInput() {
    std::memset(ssid, 0, sizeof ssid);
    std::memset(password, 0, sizeof password);
    readingPassword = false;
    invalidInput = false;
    length = 0;
}

void persistCredentials(const char* targetSsid, const char* targetPassword) {
    if (wifiPrefs.begin("cv-wifi", false)) {
        wifiPrefs.putString("ssid", targetSsid);
        wifiPrefs.putString("pass", targetPassword);
        wifiPrefs.end();
    }
}

void clearPersistedCredentials() {
    if (wifiPrefs.begin("cv-wifi", false)) {
        wifiPrefs.clear();
        wifiPrefs.end();
    }
}

void ensureSoftAp() {
    if (apActive) {
        return;
    }
    WiFi.mode(WIFI_AP_STA);
    apSsid = String("CidadeViva-") + (currentDeviceId == "esp32-water" ? "Water" : "Energy");
    apActive = WiFi.softAP(apSsid.c_str());
    if (apActive) {
        Serial.print("WIFI_AP_READY ssid=");
        Serial.print(apSsid);
        Serial.print(" ip=");
        Serial.println(WiFi.softAPIP());
    }
}

bool startWifiConnect(const char* targetSsid, const char* targetPassword, std::uint32_t now, bool persistOnSuccess) {
    const std::size_t ssidLen = std::strlen(targetSsid);
    const std::size_t passLen = std::strlen(targetPassword);
    if (ssidLen < 1 || ssidLen > 32 || (passLen != 0 && (passLen < 8 || passLen > 63))) {
        Serial.println("WIFI_TEST_INVALID_INPUT");
        lastError = "INVALID_INPUT";
        return false;
    }

    std::strncpy(ssid, targetSsid, sizeof(ssid) - 1);
    ssid[sizeof(ssid) - 1] = '\0';
    std::strncpy(password, targetPassword, sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0';

    WiFi.persistent(false);
    WiFi.mode(apActive ? WIFI_AP_STA : WIFI_STA);
    WiFi.setAutoReconnect(true);
    if (passLen == 0) {
        WiFi.begin(ssid);
    } else {
        WiFi.begin(ssid, password);
    }

    saveAfterConnect = persistOnSuccess;
    if (persistOnSuccess) {
        persistCredentials(ssid, password);
    }
    resetInput();
    startedAt = now;
    lastError = "";
    phase = Phase::Connecting;
    Serial.println("WIFI_TEST_CONNECTING");
    return true;
}

void emitSerialScan() {
    WiFi.mode(apActive ? WIFI_AP_STA : WIFI_STA);
    const int count = WiFi.scanNetworks(false, false);
    Serial.printf("WIFI_SCAN_BEGIN count=%d\n", count < 0 ? 0 : count);
    for (int i = 0; i < count; ++i) {
        String netSsid = WiFi.SSID(i);
        netSsid.replace("\r", "");
        netSsid.replace("\n", "");
        if (netSsid.length() == 0) {
            continue;
        }
        const int rssi = WiFi.RSSI(i);
        const int channel = WiFi.channel(i);
        const bool open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
        Serial.printf("WIFI_SCAN_ITEM ssid=%s|rssi=%d|ch=%d|auth=%s\n",
                      netSsid.c_str(), rssi, channel, open ? "OPEN" : "SECURED");
    }
    Serial.println("WIFI_SCAN_END");
    WiFi.scanDelete();
}

void emitSerialStatus() {
    const auto ip = WiFi.localIP();
    const bool connected = WiFi.status() == WL_CONNECTED && ip != IPAddress(0, 0, 0, 0);
    Serial.printf("WIFI_STATUS deviceId=%s phase=%s connected=%s ssid=%s ip=%s rssi=%d apSsid=%s apIp=%s\n",
                  currentDeviceId.c_str(),
                  phaseLabel(),
                  connected ? "true" : "false",
                  connected ? WiFi.SSID().c_str() : "",
                  connected ? ip.toString().c_str() : "0.0.0.0",
                  connected ? static_cast<int>(WiFi.RSSI()) : 0,
                  apSsid.c_str(),
                  apActive ? WiFi.softAPIP().toString().c_str() : "0.0.0.0");
}

String buildStatusJson() {
    const auto ip = WiFi.localIP();
    const bool connected = WiFi.status() == WL_CONNECTED && ip != IPAddress(0, 0, 0, 0);
    String json = "{";
    json += "\"deviceId\":\"" + jsonEscape(currentDeviceId) + "\",";
    json += "\"phase\":\"" + String(phaseLabel()) + "\",";
    json += "\"connected\":" + String(connected ? "true" : "false") + ",";
    json += "\"ssid\":\"" + (connected ? jsonEscape(WiFi.SSID()) : String("")) + "\",";
    json += "\"ip\":\"" + (connected ? ip.toString() : String("0.0.0.0")) + "\",";
    json += "\"rssi\":" + String(connected ? static_cast<int>(WiFi.RSSI()) : 0) + ",";
    json += "\"apSsid\":\"" + jsonEscape(apSsid) + "\",";
    json += "\"apIp\":\"" + (apActive ? WiFi.softAPIP().toString() : String("0.0.0.0")) + "\",";
    json += "\"lastError\":\"" + jsonEscape(lastError) + "\"";
    json += "}";
    return json;
}

void setupHttpServer() {
    httpServer.on("/api/wifi/status", HTTP_OPTIONS, []() {
        sendCorsHeaders();
        httpServer.send(204);
    });
    httpServer.on("/api/wifi/scan", HTTP_OPTIONS, []() {
        sendCorsHeaders();
        httpServer.send(204);
    });
    httpServer.on("/api/wifi/connect", HTTP_OPTIONS, []() {
        sendCorsHeaders();
        httpServer.send(204);
    });

    httpServer.on("/api/wifi/status", HTTP_GET, []() {
        sendCorsHeaders();
        httpServer.send(200, "application/json", buildStatusJson());
    });

    httpServer.on("/api/wifi/scan", HTTP_GET, []() {
        sendCorsHeaders();
        WiFi.mode(apActive ? WIFI_AP_STA : WIFI_STA);
        const int count = WiFi.scanNetworks(false, false);
        String json = "{\"deviceId\":\"" + jsonEscape(currentDeviceId) + "\",\"networks\":[";
        bool first = true;
        for (int i = 0; i < count; ++i) {
            const String netSsid = WiFi.SSID(i);
            if (netSsid.length() == 0) {
                continue;
            }
            if (!first) {
                json += ",";
            }
            first = false;
            const int rssi = WiFi.RSSI(i);
            const int channel = WiFi.channel(i);
            const bool open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
            json += "{\"ssid\":\"" + jsonEscape(netSsid) + "\",";
            json += "\"rssi\":" + String(rssi) + ",";
            json += "\"channel\":" + String(channel) + ",";
            json += "\"auth\":\"" + String(open ? "OPEN" : "WPA2") + "\"}";
        }
        json += "]}";
        WiFi.scanDelete();
        httpServer.send(200, "application/json", json);
    });

    httpServer.on("/api/wifi/connect", HTTP_POST, []() {
        sendCorsHeaders();
        String reqSsid = httpServer.arg("ssid");
        String reqPass = httpServer.arg("password");

        if (reqSsid.length() == 0 && httpServer.hasArg("plain")) {
            const String body = httpServer.arg("plain");
            const int ssidKey = body.indexOf("\"ssid\"");
            if (ssidKey >= 0) {
                const int colon = body.indexOf(':', ssidKey);
                const int firstQuote = body.indexOf('"', colon + 1);
                const int secondQuote = body.indexOf('"', firstQuote + 1);
                if (firstQuote >= 0 && secondQuote > firstQuote) {
                    reqSsid = body.substring(firstQuote + 1, secondQuote);
                }
            }
            const int passKey = body.indexOf("\"password\"");
            if (passKey >= 0) {
                const int colon = body.indexOf(':', passKey);
                const int firstQuote = body.indexOf('"', colon + 1);
                const int secondQuote = body.indexOf('"', firstQuote + 1);
                if (firstQuote >= 0 && secondQuote > firstQuote) {
                    reqPass = body.substring(firstQuote + 1, secondQuote);
                }
            }
        }

        if (!startWifiConnect(reqSsid.c_str(), reqPass.c_str(), millis(), true)) {
            httpServer.send(400, "application/json", "{\"error\":\"INVALID_INPUT\"}");
            return;
        }
        httpServer.send(202, "application/json", buildStatusJson());
    });

    httpServer.on("/", HTTP_GET, []() {
        sendCorsHeaders();
        String html = "<!doctype html><html lang='pt-BR'><head><meta charset='utf-8'>"
                      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                      "<title>Cidade Viva Wi-Fi (" + currentDeviceId + ")</title>"
                      "<style>body{font-family:system-ui,sans-serif;background:#f5f7f2;color:#1b3028;padding:24px;max-width:460px;margin:0 auto}"
                      ".card{background:#fff;border:1px solid #dfe6df;border-radius:10px;padding:20px}"
                      "input,button{width:100%;padding:11px;margin-top:8px;border-radius:6px;border:1px solid #cdd8cf;box-sizing:border-box}"
                      "button{background:#183d32;color:#fff;font-weight:600;border:0;cursor:pointer}</style></head><body>"
                      "<div class='card'><h2>Cidade Viva · " + currentDeviceId + "</h2>"
                      "<p>Conecte a placa na mesma rede 2,4 GHz do notebook:</p>"
                      "<form method='POST' action='/api/wifi/connect'>"
                      "<label>Rede Wi-Fi (SSID)<input name='ssid' required maxlength='32'></label>"
                      "<label>Senha<input name='password' type='password' minlength='8' maxlength='63'></label>"
                      "<button type='submit'>Conectar placa</button></form></div></body></html>";
        httpServer.send(200, "text/html", html);
    });

    httpServer.begin();
}

void initWifiProvisioning(const char* deviceId, std::uint32_t now) {
    if (provisionInitialized) {
        return;
    }
    provisionInitialized = true;
    if (deviceId != nullptr && deviceId[0] != '\0') {
        currentDeviceId = deviceId;
    }
    ensureSoftAp();
    setupHttpServer();

    if (wifiPrefs.begin("cv-wifi", true)) {
        const String savedSsid = wifiPrefs.getString("ssid", "");
        const String savedPass = wifiPrefs.getString("pass", "");
        wifiPrefs.end();
        if (savedSsid.length() >= 1 && savedSsid.length() <= 32 &&
            (savedPass.length() == 0 || (savedPass.length() >= 8 && savedPass.length() <= 63))) {
            Serial.printf("WIFI_SAVED_CREDENTIALS ssid=%s\n", savedSsid.c_str());
            startWifiConnect(savedSsid.c_str(), savedPass.c_str(), now, false);
        }
    }
}

bool handleControlCommand(const char* line, std::uint32_t now) {
    if (std::strcmp(line, "__WIFI_SCAN__") == 0) {
        emitSerialScan();
        resetInput();
        return true;
    }
    if (std::strcmp(line, "__WIFI_STATUS__") == 0) {
        emitSerialStatus();
        resetInput();
        return true;
    }
    if (std::strcmp(line, "__WIFI_FORGET__") == 0) {
        clearPersistedCredentials();
        WiFi.disconnect(false, true);
        phase = Phase::Waiting;
        resetInput();
        Serial.println("WIFI_TEST_WAITING");
        return true;
    }
    (void)now;
    return false;
}

void readCredentials(std::uint32_t now) {
    for (unsigned i = 0; i < 64 && Serial.available() > 0; ++i) {
        const auto ch = static_cast<char>(Serial.read());
        auto* buffer = readingPassword ? password : ssid;
        const std::size_t capacity = readingPassword ? sizeof password : sizeof ssid;
        if (ch != '\n') {
            if (ch == '\r') {
                continue;
            }
            if (ch == '\0' || length >= capacity - 1) {
                invalidInput = true;
            } else {
                buffer[length++] = ch;
            }
            continue;
        }

        buffer[length] = '\0';
        if (!readingPassword) {
            if (handleControlCommand(ssid, now)) {
                return;
            }
            invalidInput |= length == 0;
            readingPassword = true;
            length = 0;
            continue;
        }
        if (invalidInput || (length > 0 && length < 8)) {
            resetInput();
            Serial.println("WIFI_TEST_INVALID_INPUT");
            return;
        }

        const String targetSsid = String(ssid);
        const String targetPass = String(password);
        startWifiConnect(targetSsid.c_str(), targetPass.c_str(), now, true);
        return;
    }
}
}

inline void tickWifiTest(std::uint32_t now, const char* deviceId = "esp32") {
    initWifiProvisioning(deviceId, now);
    httpServer.handleClient();

    if (phase != Phase::Waiting && Serial.available() > 0) {
        // Allow scan, status, or reconnection commands at any time over USB serial.
        readCredentials(now);
    }

    if (phase == Phase::Waiting) {
        readCredentials(now);
        if (phase == Phase::Waiting && now - lastPromptAt >= 1000u) {
            lastPromptAt = now;
            Serial.println("WIFI_TEST_WAITING");
        }
        return;
    }

    if (phase == Phase::Connecting) {
        const auto ip = WiFi.localIP();
        if (WiFi.status() == WL_CONNECTED && ip != IPAddress(0, 0, 0, 0)) {
            phase = Phase::Connected;
            lastError = "";
            Serial.print("WIFI_TEST_CONNECTED ip=");
            Serial.println(ip);
        } else if (now - startedAt >= 30000u) {
            phase = Phase::Waiting;
            lastError = "TIMEOUT";
            Serial.println("WIFI_TEST_TIMEOUT");
        }
    } else if (phase == Phase::Connected && WiFi.status() != WL_CONNECTED) {
        phase = Phase::Waiting;
        lastError = "DISCONNECTED";
        Serial.println("WIFI_TEST_DISCONNECTED");
    }
}
