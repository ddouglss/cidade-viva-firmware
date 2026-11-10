#include <Arduino.h>
#include "DisabledEnergyHardware.hpp"
#include "../../../tools/esp32-smoke/PrivateMqttTelemetry.hpp"

namespace {
class ArduinoClock final : public energy::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};
ArduinoClock clockSource;
energy::DisabledEnergyHardware hardware;
energy::EnergyRuntime runtime(clockSource, hardware);
PrivateMqttTelemetry transport("esp32-energy", "0.3.0-telemetry-only", "MANUAL");
}

void setup() {
    runtime.begin();
    transport.begin();
    Serial.println("ENERGY_TELEMETRY sensors=NOT_CONFIGURED externalOutputs=DISABLED");
}

void loop() {
    runtime.tick();
    transport.tick(millis(), "{}", "{\"mainLightOn\":false}", "CALCULATED", false);
    delay(1);
}
