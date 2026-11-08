#pragma once
#include "LightController.hpp"
#include "TrafficController.hpp"
namespace energy {
struct IClock {
    virtual ~IClock() = default;
    virtual std::uint32_t nowMs() = 0;
};
struct IEnergyHardware {
    virtual ~IEnergyHardware() = default;
    virtual LuxReading readLux() = 0;
    virtual bool crossingPressed() = 0;
    virtual void apply(const OutputIntent& intent) = 0;
};
class EnergyRuntime {
public:
    EnergyRuntime(IClock& clock, IEnergyHardware& hardware,
                  std::optional<LightConfig> lightConfig = std::nullopt,
                  std::optional<TrafficConfig> trafficConfig = std::nullopt,
                  std::uint32_t debounceMs = 0);
    void begin();
    void tick();
    bool setLightMode(LightMode mode);
    bool setManualLight(bool on);
    bool requestCrossing() { return traffic_.requestCrossing(); }
    const LightController& light() const { return light_; }
    const TrafficController& traffic() const { return traffic_; }
private:
    void apply();
    IClock& clock_;
    IEnergyHardware& hardware_;
    LightController light_;
    TrafficController traffic_;
    CrossingButton button_;
    bool begun_ = false;
};
}
