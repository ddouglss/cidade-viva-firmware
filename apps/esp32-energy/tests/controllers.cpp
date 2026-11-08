#include "EnergyRuntime.hpp"
#include "DisabledEnergyHardware.hpp"
#include "BenchLedBlink.hpp"
#include <cmath>
#include <limits>
#include <string>
#include <vector>
using namespace energy;
void check(bool ok, const char* name);
namespace {
// Synthetic fixtures only: not electrical, traffic or lighting calibration.
LightConfig lightConfig(InvalidLightPolicy policy = InvalidLightPolicy::Off) { return {10, 20, 5, 100, policy}; }
TrafficConfig trafficConfig() { return {10, 10, 10, 10, 10, 10, 10, 10, 10, true}; }
LuxReading lux(double value, std::uint32_t at, bool valid = true) {
    return {value, valid, at, ReadingOrigin::SimulatedInput};
}
struct Clock : IClock {
    std::uint32_t now = 0;
    std::uint32_t nowMs() override { return now; }
};
struct Hardware : IEnergyHardware {
    LuxReading reading = lux(0, 0);
    bool pressed = false;
    OutputIntent last;
    std::vector<std::string> calls;
    LuxReading readLux() override { calls.push_back("lux"); return reading; }
    bool crossingPressed() override { calls.push_back("button"); return pressed; }
    void apply(const OutputIntent& value) override { calls.push_back("apply"); last = value; }
};
void lights() {
    LightController missing;
    check(!missing.requestedOn() && !missing.setMode(LightMode::Auto), "light missing config refuses AUTO");
    LightController invalid(LightConfig{20, 10, 5, 100, InvalidLightPolicy::Off});
    check(!invalid.setMode(LightMode::Auto), "light invalid thresholds refuse AUTO");
    LightController nan(LightConfig{std::numeric_limits<double>::quiet_NaN(), 20, 5, 100, InvalidLightPolicy::Off});
    check(!nan.setMode(LightMode::Auto), "light NaN configuration rejected");
    LightController c(lightConfig());
    check(c.mode() == LightMode::Manual && c.setManual(true) && c.requestedOn(), "light MANUAL explicit ON");
    c.update(0, lux(-1, 0, false));
    check(c.requestedOn(), "light MANUAL independent of lux sensor");
    c.setMode(LightMode::Auto);
    check(!c.requestedOn() && c.state() == LightState::FaultSensor && !c.setManual(true), "light AUTO entry applies configured fallback");
    c.update(0, lux(1, 0));
    c.update(4, lux(1, 4));
    check(!c.requestedOn(), "light darkness waits stability");
    c.update(5, lux(1, 5));
    check(c.requestedOn(), "light darkness stable boundary ON");
    c.update(6, lux(20, 6));
    c.update(20, lux(15, 20));
    check(c.requestedOn(), "light equality and hysteresis hold ON");
    c.update(21, lux(21, 21));
    c.update(24, lux(15, 24));
    c.update(25, lux(21, 25));
    c.update(29, lux(21, 29));
    check(c.requestedOn(), "light interrupted candidate restarts stability");
    c.update(30, lux(21, 30));
    check(!c.requestedOn(), "light high stable boundary OFF");
    c.update(31, lux(0, 31));
    c.update(32, lux(std::numeric_limits<double>::infinity(), 32));
    check(c.state() == LightState::FaultSensor && !c.requestedOn(), "light infinity faults and cancels candidate");
    c.update(33, lux(0, 33));
    c.update(37, lux(0, 37));
    check(!c.requestedOn(), "light recovery needs new stable window");
    c.update(38, lux(0, 38));
    check(c.requestedOn(), "light valid recovery ON");
    c.update(139, lux(0, 38));
    check(!c.requestedOn() && c.state() == LightState::FaultSensor, "light stale reading applies fallback");
    for (const auto policy : {InvalidLightPolicy::On, InvalidLightPolicy::Hold}) {
        LightController fallback(lightConfig(policy));
        fallback.setManual(true);
        fallback.setMode(LightMode::Auto);
        fallback.update(0, lux(0, 0, false));
        check(fallback.requestedOn(), "light caller-selected ON or HOLD fallback");
    }
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 2u;
    LightController wrap(lightConfig());
    wrap.setMode(LightMode::Auto);
    wrap.update(edge, lux(0, edge));
    wrap.update(std::uint32_t(edge + 5u), lux(0, std::uint32_t(edge + 5u)));
    check(wrap.requestedOn(), "light stability across uint32 wrap");
    c.setMode(LightMode::Manual);
    c.setManual(false);
    check(!c.requestedOn() && c.state() == LightState::Manual, "light mode returns explicit manual control");
}
void traffic() {
    TrafficController missing;
    check(!missing.requestCrossing() && !missing.configured() && !hasConflictingGreens(missing.intent()), "traffic absent config stays all red");
    auto invalid = trafficConfig();
    invalid.aYellowMs = 0;
    check(!TrafficController(invalid).configured(), "traffic zero phase duration rejected");
    TrafficController c(trafficConfig());
    c.update(0);
    c.requestCrossing();
    c.requestCrossing();
    const TrafficPhase expected[] = {TrafficPhase::AGreen, TrafficPhase::AYellow, TrafficPhase::AllRed1,
        TrafficPhase::BGreen, TrafficPhase::BYellow, TrafficPhase::AllRed2, TrafficPhase::PedWalk,
        TrafficPhase::PedClearance, TrafficPhase::AGreen};
    std::uint32_t now = 0;
    bool sequence = true;
    bool protectedWalk = true;
    for (const auto phase : expected) {
        now += 10;
        c.update(now);
        sequence = sequence && c.phase() == phase && !hasConflictingGreens(c.intent());
        if (phase == TrafficPhase::PedWalk || phase == TrafficPhase::PedClearance) {
            const auto out = c.intent();
            protectedWalk = protectedWalk && out.trafficA == Signal::Red && out.trafficB == Signal::Red &&
                out.buzzerOn == (phase == TrafficPhase::PedWalk);
            c.requestCrossing();
            sequence = sequence && !c.crossingPending();
        }
    }
    check(sequence, "traffic exact safe sequence and coalesced crossing");
    check(protectedWalk, "traffic vehicles RED during walk and clearance");
    bool cycles = true;
    const TrafficPhase normal[] = {TrafficPhase::AYellow, TrafficPhase::AllRed1, TrafficPhase::BGreen,
        TrafficPhase::BYellow, TrafficPhase::AllRed2, TrafficPhase::AGreen};
    for (int cycle = 0; cycle < 3; ++cycle) {
        for (const auto phase : normal) {
            now += 10;
            c.update(now);
            cycles = cycles && c.phase() == phase && !hasConflictingGreens(c.intent());
        }
    }
    check(cycles, "traffic three full cycles without conflicting greens");
    c.update(now + 10000);
    check(c.phase() == TrafficPhase::AYellow, "traffic delayed tick never skips safety phase");
    c.update(now + 10000);
    check(c.phase() == TrafficPhase::AYellow, "traffic same tick cannot collapse next phase");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 5u;
    TrafficController wrap(trafficConfig());
    wrap.update(edge);
    wrap.update(std::uint32_t(edge + 9u));
    check(wrap.phase() == TrafficPhase::AllRedStart, "traffic wrap before boundary");
    wrap.update(std::uint32_t(edge + 10u));
    check(wrap.phase() == TrafficPhase::AGreen, "traffic wrap at phase boundary");
    c = TrafficController(trafficConfig());
    check(c.phase() == TrafficPhase::AllRedStart && !c.crossingPending(), "traffic reboot clears pending and starts RED");
    bool resetSafe = true;
    TrafficController transitions(trafficConfig());
    transitions.update(0);
    transitions.requestCrossing();
    for (std::uint32_t t = 0; t <= 90; t += 10) {
        transitions.update(t);
        TrafficController rebooted(trafficConfig());
        const auto out = rebooted.intent();
        resetSafe = resetSafe && out.trafficA == Signal::Red && out.trafficB == Signal::Red && !out.buzzerOn;
    }
    check(resetSafe, "traffic fresh boot safe during every phase including pedestrian transition");
    std::uint32_t random = 99;
    bool invariant = true;
    now = 0;
    for (int i = 0; i < 20000; ++i) {
        random = random * 1664525u + 1013904223u;
        now += random % 50u;
        if (random % 7u == 0) c.requestCrossing();
        c.update(now);
        const auto out = c.intent();
        invariant = invariant && !hasConflictingGreens(out);
        if (c.pedestrianWalk()) invariant = invariant && out.trafficA == Signal::Red && out.trafficB == Signal::Red;
    }
    check(invariant, "traffic invariant over 20000 deterministic delayed updates");
    auto silentConfig = trafficConfig();
    silentConfig.buzzerDuringWalk = false;
    TrafficController silent(silentConfig);
    silent.update(0);
    silent.requestCrossing();
    for (std::uint32_t t = 10; t <= 70; t += 10) silent.update(t);
    check(silent.pedestrianWalk() && !silent.intent().buzzerOn, "traffic buzzer requires explicit enabled policy");
}
void buttons() {
    CrossingButton button(5);
    check(!button.update(0, true) && !button.update(10, true), "button held at boot emits no request");
    button.update(11, false);
    button.update(16, false);
    button.update(17, true);
    button.update(19, false);
    button.update(20, true);
    check(!button.update(24, true) && button.update(25, true) && !button.update(30, true), "button bounce filtered and hold emits once");
    button.update(31, false);
    button.update(36, false);
    button.update(37, true);
    check(button.update(42, true), "button debounced release rearms");
    CrossingButton disabled(0);
    check(!disabled.configured() && !disabled.update(0, false) && !disabled.update(100, true), "button missing debounce disabled");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 2u;
    CrossingButton wrap(5);
    wrap.update(edge - 1u, false);
    wrap.update(edge, true);
    check(wrap.update(std::uint32_t(edge + 5u), true), "button debounce across uint32 wrap");
}
void runtime() {
    Clock clock;
    Hardware hw;
    EnergyRuntime runtime(clock, hw, lightConfig(), trafficConfig(), 5);
    runtime.tick();
    check(hw.calls.front() == "apply" && !hw.last.mainLightOn && hw.last.trafficA == Signal::Red,
          "energy runtime applies boot before first input");
    runtime.setLightMode(LightMode::Auto);
    runtime.tick();
    clock.now = 5;
    hw.reading = lux(0, 5);
    runtime.tick();
    check(hw.last.mainLightOn, "energy runtime routes sensor to light output");
    clock.now = 10;
    runtime.tick();
    check(hw.last.trafficA == Signal::Green && !hasConflictingGreens(hw.last), "energy runtime advances local traffic without network");
    runtime.setLightMode(LightMode::Manual);
    runtime.setManualLight(false);
    check(!hw.last.mainLightOn, "energy runtime applies manual OFF immediately");
    Hardware other;
    EnergyRuntime unconfigured(clock, other);
    unconfigured.tick();
    check(!unconfigured.setLightMode(LightMode::Auto) && other.last.trafficA == Signal::Red &&
          other.last.trafficB == Signal::Red, "energy runtime absent parameters stays conservative");
    DisabledEnergyHardware disabled;
    check(!disabled.externalOutputsEnabled() && !disabled.readLux().valid && !disabled.crossingPressed(),
          "disabled adapter exposes absent sensor and button without fabricated data");
    EnergyRuntime safe(clock, disabled);
    safe.begin();
    safe.tick();
    check(!safe.light().requestedOn() && safe.light().mode() == LightMode::Manual &&
          safe.traffic().phase() == TrafficPhase::AllRedStart,
          "disabled firmware runtime boots MANUAL OFF and logical ALL_RED");
    check(!safe.setLightMode(LightMode::Auto) && !safe.requestCrossing(),
          "disabled firmware absent calibration prevents automatic light or traffic start");
    safe.setManualLight(true);
    disabled.apply(OutputIntent{true, true, Signal::Green, Signal::Green});
    check(safe.light().requestedOn() && !disabled.externalOutputsEnabled(),
          "logical requests never enable external outputs in disabled adapter");
}
}
void controllerTests() {
    lights(); traffic(); buttons(); runtime();
    BenchLedBlink blink(100);
    check(!blink.requestedHigh() && !blink.update(599), "bench LED starts LOW until diagnostic interval");
    check(blink.update(600) && blink.requestedHigh(), "bench LED HIGH at 500ms boundary");
    check(!blink.update(1099) && blink.update(1100) && !blink.requestedHigh(), "bench LED LOW on next interval");
    check(blink.update(9999) && blink.requestedHigh() && !blink.update(9999), "bench LED delayed tick advances once without busy loop");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 100u;
    BenchLedBlink wrap(edge);
    check(wrap.update(std::uint32_t(edge + 500u)) && wrap.requestedHigh(), "bench LED timer handles uint32 wrap");
}
