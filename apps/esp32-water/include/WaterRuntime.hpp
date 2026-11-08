#pragma once
#include "IrrigationController.hpp"
namespace water {
struct IClock {
    virtual ~IClock() = default;
    virtual std::uint32_t nowMs() = 0;
};
struct IFloatSwitch {
    virtual ~IFloatSwitch() = default;
    virtual Reservoir read() = 0;
};
struct ISoilSensor {
    virtual ~ISoilSensor() = default;
    virtual SoilReading read() = 0;
};
struct IPumpDriver {
    virtual ~IPumpDriver() = default;
    virtual void apply(bool requestedOn) = 0;
};
class WaterRuntime {
public:
    WaterRuntime(IClock& clock, IFloatSwitch& reservoir, ISoilSensor& soil, IPumpDriver& pump,
                 std::optional<AutoConfig> config = std::nullopt, LabLimits limits = {});
    void begin();
    void tick();
    bool setMode(IrrigationMode mode);
    RequestResult manualPulse(std::uint32_t durationMs);
    const IrrigationController& controller() const { return controller_; }
private:
    IClock& clock_;
    IFloatSwitch& reservoir_;
    ISoilSensor& soil_;
    IPumpDriver& pump_;
    IrrigationController controller_;
    bool begun_ = false;
};
}
