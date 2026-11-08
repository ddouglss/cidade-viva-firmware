#pragma once
#include "EnergySafety.hpp"
#include <cstdint>
#include <optional>
namespace energy {
enum class TrafficPhase { AllRedStart, AGreen, AYellow, AllRed1, BGreen, BYellow, AllRed2, PedWalk, PedClearance };
struct TrafficConfig {
    std::uint32_t startAllRedMs;
    std::uint32_t aGreenMs;
    std::uint32_t aYellowMs;
    std::uint32_t allRed1Ms;
    std::uint32_t bGreenMs;
    std::uint32_t bYellowMs;
    std::uint32_t allRed2Ms;
    std::uint32_t pedWalkMs;
    std::uint32_t pedClearanceMs;
    bool buzzerDuringWalk;
};
class TrafficController {
public:
    explicit TrafficController(std::optional<TrafficConfig> config = std::nullopt);
    bool requestCrossing();
    void update(std::uint32_t now);
    OutputIntent intent() const;
    TrafficPhase phase() const { return phase_; }
    bool configured() const { return config_.has_value(); }
    bool crossingPending() const { return crossingPending_; }
    bool pedestrianWalk() const { return phase_ == TrafficPhase::PedWalk; }
private:
    std::uint32_t duration() const;
    std::optional<TrafficConfig> config_;
    TrafficPhase phase_ = TrafficPhase::AllRedStart;
    bool crossingPending_ = false;
    bool started_ = false;
    std::uint32_t entered_ = 0;
};

class CrossingButton {
public:
    explicit CrossingButton(std::uint32_t debounceMs);
    bool update(std::uint32_t now, bool pressed);
    bool configured() const { return debounceMs_ > 0 && debounceMs_ < 0x80000000u; }
private:
    std::uint32_t debounceMs_;
    bool initialized_ = false;
    bool raw_ = false;
    bool stable_ = true;
    bool armed_ = false;
    std::uint32_t changed_ = 0;
};
}
