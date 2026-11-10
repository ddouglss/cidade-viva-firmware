#include <Arduino.h>
#include "DisabledWaterHardware.hpp"
#include "FloatSafetyInput.hpp"
#include "../../../tools/esp32-smoke/PrivateMqttTelemetry.hpp"

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
    water::Reservoir level() const { return input_.level(); }
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
water::DisabledPumpDriver pump;
water::WaterRuntime runtime(clockSource, floatSwitch, soilSensor, pump);
PrivateMqttTelemetry transport("esp32-water", "0.3.0-telemetry-only", "MANUAL");
String measurements = "{}";
bool measurementsValid = false;
uint32_t lastSampleAt = 0;
}

void setup() {
    runtime.begin(); // Keeps pump off before networking.
    pinMode(floatGpio, INPUT_PULLUP);
    pinMode(soilGpio, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(soilGpio, ADC_11db);
    runtime.tick();
    transport.begin();
    Serial.println("WATER_TELEMETRY pumpOutput=DISABLED calibration=NONE");
}

void loop() {
    runtime.tick();
    const uint32_t now = millis();
    if (lastSampleAt == 0 || uint32_t(now - lastSampleAt) >= 1000u) {
        lastSampleAt = now;
        const auto reading = soilSensor.read();
        const auto level = floatSwitch.level();
        measurements = "{";
        measurementsValid = reading.raw().has_value();
        if (measurementsValid) measurements += String("\"soilAdcRaw\":") + *reading.raw();
        if (level != water::Reservoir::Unknown) {
            if (measurementsValid) measurements += ",";
            measurements += String("\"reservoirLow\":")
                + (level == water::Reservoir::Low ? "true" : "false");
        } else measurementsValid = false;
        measurements += "}";
    }
    transport.tick(now, measurements, "{\"pumpOn\":false}", "PHYSICAL_SENSOR", measurementsValid);
    delay(1);
}
