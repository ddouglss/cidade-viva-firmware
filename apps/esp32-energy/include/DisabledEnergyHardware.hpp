#pragma once
#include "EnergyRuntime.hpp"

namespace energy {
// No GPIO, I2C, ADC or driver access. Intent is deliberately never applied.
// Replace only after an approved per-signal hardware mapping exists.
class DisabledEnergyHardware final : public IEnergyHardware {
public:
    LuxReading readLux() override { return {}; } // Absent: invalid, no lux measurement.
    bool crossingPressed() override { return false; }
    void apply(const OutputIntent&) override {} // No physical output, including no LED.
    static constexpr bool externalOutputsEnabled() { return false; }
};
}
