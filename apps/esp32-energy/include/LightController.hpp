#pragma once
#include <cstdint>
#include <optional>

namespace energy {
enum class LightMode { Manual, Auto };
enum class InvalidLightPolicy { Off, On, Hold };
enum class LightState { Manual, Auto, FaultSensor, InvalidConfig };
enum class ReadingOrigin { PhysicalSensor, SimulatedInput };
struct LuxReading {
    double lux = 0;
    bool valid = false;
    std::uint32_t observedAt = 0;
    ReadingOrigin origin = ReadingOrigin::PhysicalSensor;
};
struct LightConfig {
    double onBelowLux;
    double offAboveLux;
    std::uint32_t stableMs;
    std::uint32_t maxAgeMs;
    InvalidLightPolicy invalidPolicy;
};
class LightController {
public:
    explicit LightController(std::optional<LightConfig> config = std::nullopt);
    bool setMode(LightMode mode);
    bool setManual(bool on);
    void update(std::uint32_t now, const LuxReading& reading);
    bool requestedOn() const { return on_; }
    LightMode mode() const { return mode_; }
    LightState state() const { return state_; }
private:
    std::optional<LightConfig> config_;
    LightMode mode_ = LightMode::Manual;
    LightState state_ = LightState::Manual;
    bool on_ = false;
    std::optional<bool> candidate_;
    std::uint32_t candidateSince_ = 0;
};
}
