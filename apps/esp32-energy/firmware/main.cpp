#include <Arduino.h>
#include "DisabledEnergyHardware.hpp"

namespace {
class ArduinoClock final : public energy::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};

ArduinoClock clockSource;
energy::DisabledEnergyHardware disabledHardware;
// No calibrated parameters: light stays MANUAL/OFF, traffic intent stays ALL_RED.
energy::EnergyRuntime runtime(clockSource, disabledHardware);
constexpr std::uint32_t diagnosticIntervalMs = 1000;
std::uint32_t lastDiagnosticAt = 0;
}

void setup() {
    runtime.begin(); // Disabled adapter: no external pin configuration or actuation.
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_ENERGY v0.1.0-disabled");
    Serial.println("deviceId=esp32-energy externalOutputs=DISABLED");
    Serial.println("hardware=PENDING_HARDWARE_VALIDATION sensors=NOT_CONFIGURED");
    Serial.println("No application GPIO, WiFi or MQTT configured; no physical measurements.");
    Serial.printf("chip=%s cpuMHz=%u flashBytes=%u\n", ESP.getChipModel(),
                  static_cast<unsigned>(ESP.getCpuFreqMHz()),
                  static_cast<unsigned>(ESP.getFlashChipSize()));
}

void loop() {
    runtime.tick();
    const auto now = clockSource.nowMs();
    if (std::uint32_t(now - lastDiagnosticAt) >= diagnosticIntervalMs) {
        lastDiagnosticAt = now;
        Serial.printf("ENERGY_ALIVE uptimeMs=%lu externalOutputs=DISABLED "
                      "lightMode=MANUAL lightIntent=OFF trafficIntent=ALL_RED measurements=NONE\n",
                      static_cast<unsigned long>(now));
    }
    delay(1); // Yield; no blocking sensor/network work in this profile.
}
