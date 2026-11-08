#include "LightController.hpp"
#include <cmath>
namespace energy {
LightController::LightController(std::optional<LightConfig> config) : config_(config) {
    if (!config_) return;
    const auto& c = *config_;
    if (!std::isfinite(c.onBelowLux) || !std::isfinite(c.offAboveLux) ||
        c.onBelowLux < 0 || c.offAboveLux <= c.onBelowLux ||
        c.stableMs == 0 || c.stableMs >= 0x80000000u ||
        c.maxAgeMs == 0 || c.maxAgeMs >= 0x80000000u ||
        (c.invalidPolicy != InvalidLightPolicy::Off && c.invalidPolicy != InvalidLightPolicy::On &&
         c.invalidPolicy != InvalidLightPolicy::Hold)) config_.reset();
}
bool LightController::setMode(LightMode mode) {
    if (mode != LightMode::Manual && mode != LightMode::Auto) return false;
    if (mode == LightMode::Auto && !config_) return false;
    if (mode == mode_) return true;
    mode_ = mode;
    candidate_.reset();
    if (mode_ == LightMode::Manual) state_ = LightState::Manual;
    else {
        state_ = LightState::FaultSensor;
        if (config_->invalidPolicy != InvalidLightPolicy::Hold)
            on_ = config_->invalidPolicy == InvalidLightPolicy::On;
    }
    return true;
}
bool LightController::setManual(bool on) {
    if (mode_ != LightMode::Manual) return false;
    on_ = on;
    return true;
}
void LightController::update(std::uint32_t now, const LuxReading& reading) {
    if (mode_ == LightMode::Manual) return;
    const auto& c = *config_;
    if (!reading.valid || !std::isfinite(reading.lux) || reading.lux < 0 ||
        std::uint32_t(now - reading.observedAt) > c.maxAgeMs) {
        state_ = LightState::FaultSensor;
        candidate_.reset();
        if (c.invalidPolicy != InvalidLightPolicy::Hold) on_ = c.invalidPolicy == InvalidLightPolicy::On;
        return;
    }
    state_ = LightState::Auto;
    bool wanted = on_;
    if (reading.lux < c.onBelowLux) wanted = true;
    else if (reading.lux > c.offAboveLux) wanted = false;
    if (wanted == on_) { candidate_.reset(); return; }
    if (!candidate_ || *candidate_ != wanted) {
        candidate_ = wanted;
        candidateSince_ = now;
    } else if (std::uint32_t(now - candidateSince_) >= c.stableMs) {
        on_ = wanted;
        candidate_.reset();
    }
}
}
