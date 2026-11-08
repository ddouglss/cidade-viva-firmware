#include "IrrigationController.hpp"
#include <cmath>
namespace water {
IrrigationController::IrrigationController(std::optional<AutoConfig> config, LabLimits limits)
    : safety_(limits), config_(config) {
    if (!config_) return;
    const auto& c = *config_;
    if (!std::isfinite(c.dryBelowPct) || c.dryBelowPct < 0 || c.dryBelowPct > 100 ||
        c.pulseMs == 0 || c.pulseMs >= limits.maxPulseMs ||
        c.maxReadingAgeMs == 0 || c.maxReadingAgeMs >= 0x80000000u ||
        safety_.state() == State::InvalidConfig) config_.reset();
}
bool IrrigationController::setMode(std::uint32_t now, IrrigationMode mode) {
    if (mode != IrrigationMode::Manual && mode != IrrigationMode::Auto) return false;
    if (mode == IrrigationMode::Auto && !config_) return false;
    if (mode == mode_) return true;
    safety_.cancel(now);
    mode_ = mode;
    sensorFault_ = mode == IrrigationMode::Auto;
    return true;
}
RequestResult IrrigationController::manualPulse(std::uint32_t now, Reservoir reservoir, std::uint32_t durationMs) {
    safety_.evaluate(now, reservoir);
    if (mode_ != IrrigationMode::Manual) return RequestResult::WrongMode;
    return safety_.requestPulse(now, reservoir, durationMs);
}
void IrrigationController::update(std::uint32_t now, Reservoir reservoir, const SoilReading& reading) {
    safety_.evaluate(now, reservoir);
    if (mode_ != IrrigationMode::Auto) return;
    sensorFault_ = reading.quality(now, config_->maxReadingAgeMs) != Quality::Valid;
    if (sensorFault_) { safety_.cancel(now); return; }
    if (safety_.state() == State::Idle && *reading.relativePct() < config_->dryBelowPct)
        safety_.requestPulse(now, reservoir, config_->pulseMs);
}
IrrigationState IrrigationController::state() const {
    switch (safety_.state()) {
    case State::InvalidConfig: return IrrigationState::InvalidConfig;
    case State::FaultTimeout: return IrrigationState::FaultTimeout;
    case State::BlockedLowWater: return IrrigationState::BlockedLowWater;
    case State::FaultSensor: return IrrigationState::FaultSensor;
    default: break;
    }
    if (mode_ == IrrigationMode::Auto && sensorFault_) return IrrigationState::FaultSensor;
    switch (safety_.state()) {
    case State::Pumping: return IrrigationState::Pumping;
    case State::Cooldown: return IrrigationState::Cooldown;
    default: return mode_ == IrrigationMode::Auto ? IrrigationState::AutoArmed : IrrigationState::Idle;
    }
}
}
