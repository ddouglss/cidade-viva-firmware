#pragma once
#include <cstdint>

namespace water {
enum class Reservoir { Present, Low, Unknown };
enum class State { Idle, Pumping, Cooldown, BlockedLowWater, FaultSensor, FaultTimeout, InvalidConfig };
enum class RequestResult { Started, Busy, UnsafeReservoir, CoolingDown, TimeoutLatched, InvalidDuration, InvalidConfig, WrongMode };
struct LabLimits {
    std::uint32_t maxPulseMs = 3000;
    std::uint32_t cooldownMs = 15000;
};

// Pure desired state: no GPIO, flow measurement, networking or remote command API.
class PumpSafety {
public:
    explicit PumpSafety(LabLimits limits = {});
    void evaluate(std::uint32_t now, Reservoir reservoir);
    void cancel(std::uint32_t now); // Local OFF; never clears a latched timeout.
    RequestResult requestPulse(std::uint32_t now, Reservoir reservoir, std::uint32_t durationMs);
    bool pumpRequestedOn() const { return state_ == State::Pumping; }
    State state() const { return state_; }
private:
    void stop(std::uint32_t now);
    LabLimits limits_;
    State state_ = State::Idle;
    bool cooling_ = false;
    bool timedOut_ = false;
    std::uint32_t started_ = 0;
    std::uint32_t stopped_ = 0;
    std::uint32_t duration_ = 0;
};
}
