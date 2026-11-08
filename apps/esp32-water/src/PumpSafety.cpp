#include "PumpSafety.hpp"

namespace water {
PumpSafety::PumpSafety(LabLimits limits) : limits_(limits) {
    constexpr std::uint32_t halfRange = 0x80000000u;
    if (limits.maxPulseMs == 0 || limits.cooldownMs == 0 ||
        limits.maxPulseMs >= halfRange || limits.cooldownMs >= halfRange) {
        state_ = State::InvalidConfig;
    }
}
void PumpSafety::stop(std::uint32_t now) {
    stopped_ = now;
    cooling_ = true;
}
void PumpSafety::cancel(std::uint32_t now) {
    if (state_ != State::Pumping) return;
    stop(now);
    timedOut_ = std::uint32_t(now - started_) >= limits_.maxPulseMs;
    state_ = timedOut_ ? State::FaultTimeout : State::Cooldown;
}
void PumpSafety::evaluate(std::uint32_t now, Reservoir reservoir) {
    if (state_ == State::InvalidConfig) return;
    if (state_ == State::Pumping) {
        const auto elapsed = std::uint32_t(now - started_);
        if (elapsed >= limits_.maxPulseMs) {
            stop(now);
            timedOut_ = true;
        } else if (reservoir != Reservoir::Present || elapsed >= duration_) {
            stop(now);
        } else {
            return;
        }
    }
    if (cooling_ && std::uint32_t(now - stopped_) >= limits_.cooldownMs) cooling_ = false;
    if (timedOut_) state_ = State::FaultTimeout;
    else if (reservoir == Reservoir::Low) state_ = State::BlockedLowWater;
    else if (reservoir != Reservoir::Present) state_ = State::FaultSensor;
    else state_ = cooling_ ? State::Cooldown : State::Idle;
}
RequestResult PumpSafety::requestPulse(std::uint32_t now, Reservoir reservoir, std::uint32_t durationMs) {
    evaluate(now, reservoir); // Safety always runs before a request is considered.
    if (state_ == State::InvalidConfig) return RequestResult::InvalidConfig;
    if (timedOut_) return RequestResult::TimeoutLatched;
    if (reservoir != Reservoir::Present) return RequestResult::UnsafeReservoir;
    if (state_ == State::Pumping) return RequestResult::Busy;
    if (cooling_) return RequestResult::CoolingDown;
    if (durationMs == 0 || durationMs > limits_.maxPulseMs) return RequestResult::InvalidDuration;
    started_ = now;
    duration_ = durationMs;
    state_ = State::Pumping;
    return RequestResult::Started;
}
}
