#include <Arduino.h>
#include "DisabledWaterHardware.hpp"

namespace {
class ArduinoClock final : public water::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};
ArduinoClock clockSource;
water::UnconnectedFloatSwitch floatSwitch;
water::UnconnectedSoilSensor soilSensor;
water::DisabledPumpDriver pumpDriver;
water::WaterRuntime runtime(clockSource, floatSwitch, soilSensor, pumpDriver);
std::uint32_t lastDiagnosticAt = 0;

const char* stateName(water::IrrigationState state) {
    using water::IrrigationState;
    switch (state) {
    case IrrigationState::Idle: return "IDLE";
    case IrrigationState::AutoArmed: return "AUTO_ARMED";
    case IrrigationState::Pumping: return "PUMPING";
    case IrrigationState::Cooldown: return "COOLDOWN";
    case IrrigationState::BlockedLowWater: return "BLOCKED_LOW_WATER";
    case IrrigationState::FaultSensor: return "FAULT_SENSOR";
    case IrrigationState::FaultTimeout: return "FAULT_TIMEOUT";
    case IrrigationState::InvalidConfig: return "INVALID_CONFIG";
    }
    return "UNKNOWN";
}
}

void setup() {
    runtime.begin();
    runtime.tick(); // Absent float must block the local controller immediately.
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_WATER v0.1.0-disabled");
    Serial.println("deviceId=esp32-water externalOutputs=DISABLED pumpOutput=DISABLED");
    Serial.println("hardware=PENDING_HARDWARE_VALIDATION reservoir=UNKNOWN soilQuality=INVALID");
    Serial.println("No application GPIO, WiFi/MQTT or physical measurements configured.");
    Serial.printf("chip=%s cpuMHz=%u flashBytes=%u\n", ESP.getChipModel(),
                  static_cast<unsigned>(ESP.getCpuFreqMHz()),
                  static_cast<unsigned>(ESP.getFlashChipSize()));
}

void loop() {
    runtime.tick();
    const auto now = clockSource.nowMs();
    if (std::uint32_t(now - lastDiagnosticAt) >= 1000u) {
        lastDiagnosticAt = now;
        Serial.printf("WATER_ALIVE uptimeMs=%lu pumpRequested=%s pumpOutput=DISABLED "
                      "mode=MANUAL reservoir=UNKNOWN state=%s measurements=NONE\n",
                      static_cast<unsigned long>(now), runtime.controller().pumpRequestedOn() ? "ON" : "OFF",
                      stateName(runtime.controller().state()));
    }
    delay(1);
}
