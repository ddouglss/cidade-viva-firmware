#include "TrafficController.hpp"
#include <initializer_list>
namespace energy {
TrafficController::TrafficController(std::optional<TrafficConfig> config) : config_(config) {
    if (!config_) return;
    const auto& c = *config_;
    for (const auto ms : {c.startAllRedMs, c.aGreenMs, c.aYellowMs, c.allRed1Ms,
                         c.bGreenMs, c.bYellowMs, c.allRed2Ms, c.pedWalkMs, c.pedClearanceMs}) {
        if (ms == 0 || ms >= 0x80000000u) { config_.reset(); break; }
    }
}
bool TrafficController::requestCrossing() {
    if (!config_) return false;
    if (phase_ != TrafficPhase::PedWalk && phase_ != TrafficPhase::PedClearance) crossingPending_ = true;
    return true;
}
std::uint32_t TrafficController::duration() const {
    const auto& c = *config_;
    switch (phase_) {
    case TrafficPhase::AllRedStart: return c.startAllRedMs;
    case TrafficPhase::AGreen: return c.aGreenMs;
    case TrafficPhase::AYellow: return c.aYellowMs;
    case TrafficPhase::AllRed1: return c.allRed1Ms;
    case TrafficPhase::BGreen: return c.bGreenMs;
    case TrafficPhase::BYellow: return c.bYellowMs;
    case TrafficPhase::AllRed2: return c.allRed2Ms;
    case TrafficPhase::PedWalk: return c.pedWalkMs;
    case TrafficPhase::PedClearance: return c.pedClearanceMs;
    }
    return 0;
}
void TrafficController::update(std::uint32_t now) {
    if (!config_) return;
    if (!started_) { started_ = true; entered_ = now; return; }
    if (std::uint32_t(now - entered_) < duration()) return;
    switch (phase_) {
    case TrafficPhase::AllRedStart: phase_ = TrafficPhase::AGreen; break;
    case TrafficPhase::AGreen: phase_ = TrafficPhase::AYellow; break;
    case TrafficPhase::AYellow: phase_ = TrafficPhase::AllRed1; break;
    case TrafficPhase::AllRed1: phase_ = TrafficPhase::BGreen; break;
    case TrafficPhase::BGreen: phase_ = TrafficPhase::BYellow; break;
    case TrafficPhase::BYellow: phase_ = TrafficPhase::AllRed2; break;
    case TrafficPhase::AllRed2:
        phase_ = crossingPending_ ? TrafficPhase::PedWalk : TrafficPhase::AGreen;
        crossingPending_ = false;
        break;
    case TrafficPhase::PedWalk: phase_ = TrafficPhase::PedClearance; break;
    case TrafficPhase::PedClearance: phase_ = TrafficPhase::AGreen; break;
    }
    entered_ = now;
}
OutputIntent TrafficController::intent() const {
    OutputIntent result;
    switch (phase_) {
    case TrafficPhase::AGreen: result.trafficA = Signal::Green; break;
    case TrafficPhase::AYellow: result.trafficA = Signal::Yellow; break;
    case TrafficPhase::BGreen: result.trafficB = Signal::Green; break;
    case TrafficPhase::BYellow: result.trafficB = Signal::Yellow; break;
    default: break;
    }
    result.buzzerOn = config_ && config_->buzzerDuringWalk && pedestrianWalk();
    return result;
}
CrossingButton::CrossingButton(std::uint32_t debounceMs) : debounceMs_(debounceMs) {}
bool CrossingButton::update(std::uint32_t now, bool pressed) {
    if (!configured()) return false;
    if (!initialized_) {
        initialized_ = true;
        raw_ = pressed;
        stable_ = pressed;
        armed_ = !pressed;
        changed_ = now;
        return false;
    }
    if (pressed != raw_) { raw_ = pressed; changed_ = now; }
    if (raw_ == stable_ || std::uint32_t(now - changed_) < debounceMs_) return false;
    stable_ = raw_;
    if (!stable_) { armed_ = true; return false; }
    const bool event = armed_;
    armed_ = false;
    return event;
}
}
