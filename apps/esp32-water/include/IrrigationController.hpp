#pragma once
#include "PumpSafety.hpp"
#include "SoilReading.hpp"
#include <optional>
namespace water {
enum class IrrigationMode { Manual, Auto };
enum class IrrigationState { Idle, AutoArmed, Pumping, Cooldown, BlockedLowWater, FaultSensor, FaultTimeout, InvalidConfig };
struct AutoConfig {
    double dryBelowPct;
    std::uint32_t pulseMs;
    std::uint32_t maxReadingAgeMs;
};
class IrrigationController {
public:
    explicit IrrigationController(std::optional<AutoConfig> config = std::nullopt, LabLimits limits = {});
    bool setMode(std::uint32_t now, IrrigationMode mode);
    RequestResult manualPulse(std::uint32_t now, Reservoir reservoir, std::uint32_t durationMs);
    void evaluateSafety(std::uint32_t now, Reservoir reservoir) { safety_.evaluate(now, reservoir); }
    void update(std::uint32_t now, Reservoir reservoir, const SoilReading& reading);
    bool pumpRequestedOn() const { return safety_.pumpRequestedOn(); }
    IrrigationMode mode() const { return mode_; }
    IrrigationState state() const;
private:
    PumpSafety safety_;
    std::optional<AutoConfig> config_;
    IrrigationMode mode_ = IrrigationMode::Manual;
    bool sensorFault_ = false;
};
}
