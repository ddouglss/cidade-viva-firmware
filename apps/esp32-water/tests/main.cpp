#include "PumpSafety.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace water;
void controllerTests();
void check(bool ok, const char* name) {
    if (!ok) { std::cerr << "FAIL: " << name << '\n'; std::exit(1); }
    std::cout << "PASS: " << name << '\n';
}
int main() {
    constexpr auto present = Reservoir::Present;
    PumpSafety p;
    check(!p.pumpRequestedOn() && p.state() == State::Idle, "boot OFF");
    check(p.requestPulse(0, Reservoir::Low, 100) == RequestResult::UnsafeReservoir && !p.pumpRequestedOn(), "low blocks request");
    check(p.requestPulse(1, Reservoir::Unknown, 100) == RequestResult::UnsafeReservoir && p.state() == State::FaultSensor, "unknown blocks request");
    check(p.requestPulse(2, present, 0) == RequestResult::InvalidDuration &&
          p.requestPulse(2, present, 3001) == RequestResult::InvalidDuration, "invalid duration stays OFF");
    check(p.requestPulse(100, present, 1000) == RequestResult::Started, "valid local pulse starts");
    check(p.requestPulse(500, present, 2000) == RequestResult::Busy, "busy request cannot extend pulse");
    p.evaluate(1099, present);
    check(p.pumpRequestedOn(), "pulse active before boundary");
    p.evaluate(1100, present);
    check(!p.pumpRequestedOn() && p.state() == State::Cooldown, "duration boundary OFF");
    check(p.requestPulse(16099, present, 100) == RequestResult::CoolingDown, "cooldown before boundary");
    check(p.requestPulse(16100, present, 100) == RequestResult::Started, "cooldown recovers at boundary");
    p.evaluate(16101, Reservoir::Low);
    check(!p.pumpRequestedOn() && p.state() == State::BlockedLowWater, "low interrupts running pulse");
    check(p.requestPulse(16102, present, 100) == RequestResult::CoolingDown, "water return does not bypass cooldown");
    p.evaluate(31101, present);
    check(!p.pumpRequestedOn() && p.state() == State::Idle, "water recovery never auto starts");
    p.requestPulse(32000, present, 1000);
    p.evaluate(32001, Reservoir::Unknown);
    check(!p.pumpRequestedOn() && p.state() == State::FaultSensor, "unknown interrupts running pulse");
    PumpSafety timeout;
    timeout.requestPulse(10, present, 3000);
    timeout.evaluate(3009, present);
    check(timeout.pumpRequestedOn(), "maximum before boundary");
    timeout.evaluate(3010, present);
    check(!timeout.pumpRequestedOn() && timeout.state() == State::FaultTimeout, "maximum boundary timeout OFF");
    check(timeout.requestPulse(100000, present, 10) == RequestResult::TimeoutLatched, "timeout does not recover with time");
    PumpSafety late;
    late.requestPulse(0, present, 100);
    late.evaluate(4000, present);
    check(!late.pumpRequestedOn() && late.state() == State::FaultTimeout, "late evaluation detects local maximum");
    PumpSafety reset;
    reset.requestPulse(0, present, 1000);
    reset = PumpSafety{};
    check(!reset.pumpRequestedOn(), "new instance after reset OFF");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 50u;
    PumpSafety wrap;
    wrap.requestPulse(edge, present, 100);
    wrap.evaluate(std::uint32_t(edge + 99u), present);
    check(wrap.pumpRequestedOn(), "wrap pulse before boundary");
    wrap.evaluate(std::uint32_t(edge + 100u), present);
    check(wrap.state() == State::Cooldown, "wrap pulse end");
    check(wrap.requestPulse(std::uint32_t(edge + 15099u), present, 100) == RequestResult::CoolingDown &&
          wrap.requestPulse(std::uint32_t(edge + 15100u), present, 100) == RequestResult::Started, "wrap cooldown recovery");
    PumpSafety wrapTimeout;
    wrapTimeout.requestPulse(edge, present, 3000);
    wrapTimeout.evaluate(std::uint32_t(edge + 3000u), present);
    check(wrapTimeout.state() == State::FaultTimeout && !wrapTimeout.pumpRequestedOn(), "wrap timeout");
    PumpSafety invalid({0, 15000});
    check(invalid.requestPulse(0, present, 10) == RequestResult::InvalidConfig && !invalid.pumpRequestedOn(), "invalid config OFF");
    PumpSafety invalidRange({3000, 0x80000000u});
    check(invalidRange.state() == State::InvalidConfig, "ambiguous temporal range blocked");
    controllerTests();
}
