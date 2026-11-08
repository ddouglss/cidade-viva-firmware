#pragma once
#include "WaterRuntime.hpp"

namespace water {
class UnconnectedFloatSwitch final : public IFloatSwitch {
public:
    Reservoir read() override { return Reservoir::Unknown; }
};
class UnconnectedSoilSensor final : public ISoilSensor {
public:
    SoilReading read() override { return SoilReading::invalid(Origin::PhysicalSensor, 0); }
};
class DisabledPumpDriver final : public IPumpDriver {
public:
    void apply(bool) override {} // No electrical output, even if an intent were ON.
    static constexpr bool externalOutputsEnabled() { return false; }
};
}
