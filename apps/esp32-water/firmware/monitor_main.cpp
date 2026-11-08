#include <Arduino.h>
#include "DisabledWaterHardware.hpp"
#include "FloatSafetyInput.hpp"

namespace {
constexpr int floatGpio = 27;
constexpr int soilGpio = 34;
class ArduinoClock final : public water::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};
class PhysicalFloat final : public water::IFloatSwitch {
public:
    water::Reservoir read() override { return input_.update(millis(), digitalRead(floatGpio) == LOW); }
    water::Reservoir indicatedLevel() const { return input_.level(); }
private:
    water::FloatSafetyInput input_;
};
class PhysicalSoil final : public water::ISoilSensor {
public:
    water::SoilReading read() override {
        return water::SoilReading::physical(static_cast<int>(analogRead(soilGpio)), millis());
    }
};
ArduinoClock clockSource;
PhysicalFloat floatSwitch;
PhysicalSoil soilSensor;
water::DisabledPumpDriver disabledPump;
water::WaterRuntime runtime(clockSource, floatSwitch, soilSensor, disabledPump);
std::uint32_t lastLogAt = 0;

const char* levelName(water::Reservoir value) {
    switch (value) {
    case water::Reservoir::Present: return "PRESENT";
    case water::Reservoir::Low: return "LOW";
    default: return "UNKNOWN";
    }
}
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
    pinMode(floatGpio, INPUT_PULLUP);
    pinMode(soilGpio, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(soilGpio, ADC_11db);
    runtime.tick();
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_WATER v0.2.0-pump-disabled");
    Serial.println("deviceId=esp32-water floatInput=G27 soilInput=G34 pumpOutput=DISABLED");
    Serial.println("Float indication: stable CLOSED=PRESENT, OPEN=LOW, closing=UNKNOWN.");
    Serial.println("Installation=PENDING_VALIDATION pumpCircuit=PENDING_VALIDATION calibration=NONE");
    Serial.println("No pump GPIO, WiFi/MQTT or AUTO commands configured.");
}

void loop() {
    runtime.tick();
    const auto now = clockSource.nowMs();
    if (std::uint32_t(now - lastLogAt) >= 1000u) {
        lastLogAt = now;
        const auto reading = soilSensor.read();
        Serial.printf("WATER_STATUS uptimeMs=%lu reservoir=%s state=%s mode=MANUAL "
                      "pumpRequested=%s pumpOutput=DISABLED soilRaw=%d unit=ADC_COUNT "
                      "origin=PHYSICAL_SENSOR calibration=NONE humidityPct=UNAVAILABLE\n",
                      static_cast<unsigned long>(now), levelName(floatSwitch.indicatedLevel()),
                      stateName(runtime.controller().state()), runtime.controller().pumpRequestedOn() ? "ON" : "OFF",
                      *reading.raw());
    }
    delay(1);
}
