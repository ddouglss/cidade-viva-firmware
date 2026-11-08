#include <Arduino.h>

namespace {
constexpr uint32_t logIntervalMs = 1000;
uint32_t lastLogAt = 0;
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_USB_SMOKE v1");
    Serial.println("DIAGNOSTIC_ONLY: external actuators and sensors NOT configured");
    Serial.printf("chip=%s revision=%u cpuMHz=%u flashBytes=%u\n",
                  ESP.getChipModel(), static_cast<unsigned>(ESP.getChipRevision()),
                  static_cast<unsigned>(ESP.getCpuFreqMHz()),
                  static_cast<unsigned>(ESP.getFlashChipSize()));
    Serial.println("No WiFi/MQTT; no application GPIO configured.");
}

void loop() {
    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - lastLogAt) >= logIntervalMs) {
        lastLogAt = now;
        Serial.printf("USB_SMOKE_ALIVE uptimeMs=%lu\n", static_cast<unsigned long>(now));
    }
    delay(1); // Yield to RTOS; diagnostic firmware only.
}
