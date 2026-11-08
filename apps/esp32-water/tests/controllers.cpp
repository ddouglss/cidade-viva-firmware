#include "WaterRuntime.hpp"
#include "DisabledWaterHardware.hpp"
#include "BenchFloatContact.hpp"
#include "FloatSafetyInput.hpp"
#include <cmath>
#include <limits>
#include <string>
#include <vector>
using namespace water;
void check(bool ok, const char* name);
namespace {
// Synthetic calibration and threshold: never a physical installation profile.
AutoConfig autoConfig() { return {30, 100, 1000}; }
SoilCalibration calibration() { return {800, 200, 0, 1000}; }
void readings() {
    auto uncalibrated = SoilReading::physical(500, 0);
    check(uncalibrated.raw() == 500 && !uncalibrated.relativePct() &&
          uncalibrated.origin() == Origin::PhysicalSensor, "soil uncalibrated keeps raw without percentage");
    auto missing = SoilReading::physical(std::nullopt, 0, calibration());
    check(missing.quality(0, 100) == Quality::Invalid && !missing.raw(), "soil absent sensor never interpreted as dry");
    auto dry = SoilReading::physical(800, 0, calibration());
    auto wet = SoilReading::physical(200, 0, calibration());
    auto middle = SoilReading::physical(500, 0, calibration());
    check(dry.relativePct() == 0 && wet.relativePct() == 100 && middle.relativePct() == 50,
          "soil relative calibration endpoints and midpoint");
    auto reversed = SoilReading::physical(500, 0, SoilCalibration{200, 800, 0, 1000});
    check(reversed.relativePct() == 50, "soil opposite calibration polarity supported");
    check(!SoilReading::physical(801, 0, calibration()).relativePct() &&
          !SoilReading::physical(199, 0, calibration()).relativePct(), "soil outside calibrated interval rejected without clamping");
    check(!SoilReading::physical(500, 0, SoilCalibration{500, 500, 0, 1000}).relativePct() &&
          !SoilReading::physical(500, 0, SoilCalibration{800, 200, 300, 1000}).relativePct(),
          "soil degenerate and out-of-range calibration rejected");
    const auto simulated = SoilReading::simulated(0, 10);
    check(simulated.origin() == Origin::SimulatedInput && !simulated.raw() && simulated.relativePct() == 0,
          "soil simulation explicitly distinct from physical ADC");
    check(simulated.quality(110, 100) == Quality::Valid && simulated.quality(111, 100) == Quality::Stale,
          "soil sample age exact boundary");
    check(SoilReading::simulated(std::numeric_limits<double>::quiet_NaN(), 0).quality(0, 100) == Quality::Invalid &&
          !SoilReading::simulated(-1, 0).relativePct() && !SoilReading::simulated(101, 0).relativePct() &&
          !SoilReading::simulated(std::numeric_limits<double>::infinity(), 0).relativePct(),
          "soil NaN infinity and invalid percentages rejected");
    check(simulated.quality(0, 100) == Quality::Stale && simulated.quality(10, 0) == Quality::Invalid,
          "soil future time and absent age policy cannot drive AUTO");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 5u;
    auto wrap = SoilReading::simulated(20, edge);
    check(wrap.quality(std::uint32_t(edge + 10u), 10) == Quality::Valid &&
          wrap.quality(std::uint32_t(edge + 11u), 10) == Quality::Stale, "soil freshness across uint32 wrap");
}
void irrigation() {
    constexpr auto present = Reservoir::Present;
    IrrigationController missing;
    check(!missing.setMode(0, IrrigationMode::Auto) && !missing.pumpRequestedOn(), "irrigation missing config blocks AUTO");
    IrrigationController invalid(AutoConfig{30, 3000, 100});
    check(!invalid.setMode(0, IrrigationMode::Auto), "irrigation AUTO pulse must stay below safety maximum");
    IrrigationController nan(AutoConfig{std::numeric_limits<double>::quiet_NaN(), 100, 100});
    check(!nan.setMode(0, IrrigationMode::Auto), "irrigation NaN threshold refused");
    IrrigationController c(autoConfig());
    check(c.mode() == IrrigationMode::Manual && !c.pumpRequestedOn(), "irrigation boot manual and OFF");
    c.setMode(0, IrrigationMode::Auto);
    c.update(0, present, SoilReading::physical(500, 0));
    check(c.state() == IrrigationState::FaultSensor && !c.pumpRequestedOn(), "irrigation uncalibrated physical input inhibits AUTO");
    c.update(1, present, SoilReading::simulated(30, 1));
    check(c.state() == IrrigationState::AutoArmed && !c.pumpRequestedOn(), "irrigation threshold equality does not mean dry");
    c.update(2, present, SoilReading::simulated(10, 2));
    check(c.pumpRequestedOn() && c.state() == IrrigationState::Pumping, "irrigation labeled dry simulation starts local AUTO");
    check(c.manualPulse(3, present, 100) == RequestResult::WrongMode, "irrigation manual request cannot bypass AUTO mode");
    c.update(101, present, SoilReading::simulated(90, 101));
    check(c.pumpRequestedOn(), "irrigation accepted pulse remains bounded despite wet reading");
    c.update(102, present, SoilReading::simulated(90, 102));
    check(!c.pumpRequestedOn() && c.state() == IrrigationState::Cooldown, "irrigation AUTO pulse ends at configured duration");
    c.update(15101, present, SoilReading::simulated(10, 15101));
    check(!c.pumpRequestedOn(), "irrigation dry cannot bypass cooldown");
    c.update(15102, present, SoilReading::simulated(10, 15102));
    check(c.pumpRequestedOn(), "irrigation fresh dry decision after cooldown");
    c.update(15103, Reservoir::Low, SoilReading::simulated(10, 15103));
    check(!c.pumpRequestedOn() && c.state() == IrrigationState::BlockedLowWater, "irrigation AUTO low reservoir overrides sensor");
    c.update(30103, present, SoilReading::simulated(90, 30103));
    check(!c.pumpRequestedOn(), "irrigation wet recovery cannot restart pump");
    c.update(30104, present, SoilReading::simulated(10, 30104));
    c.update(30105, present, SoilReading::invalid(Origin::SimulatedInput, 30105));
    check(!c.pumpRequestedOn() && c.state() == IrrigationState::FaultSensor, "irrigation invalid input stops running AUTO");
    c.update(45105, present, SoilReading::simulated(10, 0));
    check(!c.pumpRequestedOn() && c.state() == IrrigationState::FaultSensor, "irrigation stale dry input cannot restart");
    c.update(45106, present, SoilReading::physical(800, 45106, calibration()));
    check(c.pumpRequestedOn(), "irrigation calibrated physical dry input starts AUTO");
    c.setMode(45107, IrrigationMode::Manual);
    check(!c.pumpRequestedOn() && c.manualPulse(45108, present, 100) == RequestResult::CoolingDown,
          "irrigation mode change OFF preserves cooldown");
    check(c.manualPulse(60107, present, 100) == RequestResult::Started, "irrigation manual recovers only after cooldown");
    c.setMode(60108, IrrigationMode::Manual);
    c.update(60206, present, SoilReading::invalid(Origin::PhysicalSensor, 60206));
    check(c.pumpRequestedOn(), "irrigation same mode does not cancel and manual does not require soil");
    c.update(60207, present, SoilReading::invalid(Origin::PhysicalSensor, 60207));
    check(!c.pumpRequestedOn(), "irrigation manual duration still enforced without soil");
    IrrigationController timeout(autoConfig());
    timeout.setMode(0, IrrigationMode::Auto);
    timeout.update(0, present, SoilReading::simulated(0, 0));
    timeout.update(3000, present, SoilReading::simulated(0, 3000));
    timeout.setMode(3001, IrrigationMode::Manual);
    check(timeout.state() == IrrigationState::FaultTimeout &&
          timeout.manualPulse(100000, present, 100) == RequestResult::TimeoutLatched,
          "irrigation mode switch cannot clear timeout latch");
    IrrigationController unknown(autoConfig());
    unknown.setMode(0, IrrigationMode::Auto);
    unknown.update(0, Reservoir::Unknown, SoilReading::simulated(0, 0));
    check(!unknown.pumpRequestedOn() && unknown.state() == IrrigationState::FaultSensor,
          "irrigation unknown float blocks otherwise valid AUTO");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 50u;
    IrrigationController wrap(autoConfig());
    wrap.setMode(edge, IrrigationMode::Auto);
    wrap.update(edge, present, SoilReading::simulated(0, edge));
    wrap.update(std::uint32_t(edge + 100u), present, SoilReading::simulated(0, std::uint32_t(edge + 100u)));
    check(wrap.state() == IrrigationState::Cooldown && !wrap.pumpRequestedOn(), "irrigation AUTO across uint32 wrap");
    PumpSafety canceled;
    canceled.requestPulse(0, present, 100);
    canceled.cancel(10);
    canceled.cancel(1000);
    check(canceled.requestPulse(15010, present, 100) == RequestResult::Started, "local repeated OFF does not extend cooldown");
    IrrigationController lateMode(autoConfig());
    lateMode.manualPulse(0, present, 100);
    lateMode.setMode(3000, IrrigationMode::Auto);
    check(lateMode.state() == IrrigationState::FaultTimeout && !lateMode.pumpRequestedOn(),
          "irrigation late mode change cannot hide elapsed timeout");
    IrrigationController stale(AutoConfig{30, 100, 10});
    stale.setMode(0, IrrigationMode::Auto);
    const auto sample = SoilReading::simulated(0, 0);
    stale.update(0, present, sample);
    stale.update(11, present, sample);
    check(!stale.pumpRequestedOn() && stale.state() == IrrigationState::FaultSensor,
          "irrigation reading becoming stale interrupts active AUTO pulse");
    PumpSafety malformed;
    malformed.evaluate(0, static_cast<Reservoir>(99));
    check(malformed.state() == State::FaultSensor && !malformed.pumpRequestedOn(),
          "unrecognized internal float state fails closed");
}
struct FakeHardware : IClock, IFloatSwitch, IPumpDriver {
    std::uint32_t now = 0;
    Reservoir reservoir = Reservoir::Present;
    bool on = true; // Fake initial condition deliberately unsafe, begin must clear it.
    bool changeReservoirDuringSoil = false;
    std::vector<std::string> calls;
    std::uint32_t nowMs() override { return now; }
    void apply(bool value) override { calls.push_back(value ? "ON" : "OFF"); on = value; }
    Reservoir read() override { calls.push_back("float"); return reservoir; }
};
struct Soil : ISoilSensor {
    FakeHardware& hw;
    explicit Soil(FakeHardware& hardware) : hw(hardware) {}
    SoilReading read() override {
        hw.calls.push_back("soil");
        if (hw.changeReservoirDuringSoil) hw.reservoir = Reservoir::Low;
        return SoilReading::simulated(0, hw.now);
    }
};
void runtime() {
    FakeHardware hw;
    Soil soil(hw);
    WaterRuntime r(hw, hw, soil, hw, autoConfig());
    r.tick();
    check(hw.calls.front() == "OFF" && !hw.on && hw.calls.size() == 3,
          "water runtime OFF before sensor read and MANUAL skips soil");
    check(r.manualPulse(100) == RequestResult::Started && hw.on, "water runtime applies accepted manual pulse");
    hw.now = 1;
    hw.reservoir = Reservoir::Unknown;
    r.tick();
    check(!hw.on && r.controller().state() == IrrigationState::FaultSensor,
          "water runtime unknown float applies OFF");
    hw.reservoir = Reservoir::Present;
    hw.now = 15001;
    r.setMode(IrrigationMode::Auto);
    r.tick();
    check(hw.on, "water runtime fresh simulated AUTO drives fake output");
    hw.reservoir = Reservoir::Low;
    hw.now = 15002;
    hw.calls.clear();
    r.tick();
    check(hw.calls.size() == 5 && hw.calls[0] == "float" && hw.calls[1] == "OFF" && hw.calls[2] == "soil" && !hw.on,
          "water runtime applies low-water OFF before soil acquisition");
    FakeHardware changed;
    Soil changingSoil(changed);
    WaterRuntime second(changed, changed, changingSoil, changed, autoConfig());
    second.setMode(IrrigationMode::Auto);
    changed.changeReservoirDuringSoil = true;
    second.tick();
    check(!changed.on && second.controller().state() == IrrigationState::BlockedLowWater,
          "water runtime rechecks float after soil before starting pulse");
    FakeHardware reset;
    Soil resetSoil(reset);
    WaterRuntime restarted(reset, reset, resetSoil, reset, autoConfig());
    restarted.begin();
    check(!reset.on && restarted.controller().mode() == IrrigationMode::Manual,
          "water runtime new boot applies OFF with no automatic restart");
}
}
void controllerTests() {
    readings(); irrigation(); runtime();
    FakeHardware clock;
    UnconnectedFloatSwitch missingFloat;
    UnconnectedSoilSensor missingSoil;
    DisabledPumpDriver disabledPump;
    check(missingFloat.read() == Reservoir::Unknown && !missingSoil.read().relativePct(),
          "water disabled hardware never invents water present or humidity");
    WaterRuntime disabled(clock, missingFloat, missingSoil, disabledPump);
    disabled.begin();
    disabled.tick();
    check(!disabled.controller().pumpRequestedOn() && disabled.controller().mode() == IrrigationMode::Manual &&
          disabled.controller().state() == IrrigationState::FaultSensor,
          "water disabled runtime boots OFF and unknown float blocks control");
    check(disabled.manualPulse(100) == RequestResult::UnsafeReservoir && !disabled.controller().pumpRequestedOn(),
          "water disabled runtime refuses manual pulse with absent float");
    check(!disabled.setMode(IrrigationMode::Auto), "water disabled profile refuses AUTO without configuration");
    disabledPump.apply(true);
    check(!disabledPump.externalOutputsEnabled(), "water disabled driver cannot enable external pump output");
    BenchFloatContact contact;
    contact.update(100, false);
    check(!contact.stableClosed(), "float bench starts SETTLING rather than guessing water");
    contact.update(149, false);
    check(!contact.stableClosed(), "float bench waits full diagnostic debounce");
    contact.update(150, false);
    check(contact.stableClosed() == false, "float bench stable open contact at boundary");
    contact.update(160, true);
    contact.update(180, false);
    contact.update(190, true);
    contact.update(239, true);
    check(!contact.stableClosed(), "float bench bouncing contact resets debounce");
    contact.update(240, true);
    check(contact.stableClosed() == true, "float bench reports stable closed without level mapping");
    constexpr auto edge = std::numeric_limits<std::uint32_t>::max() - 10u;
    BenchFloatContact wrap;
    wrap.update(edge, true);
    wrap.update(std::uint32_t(edge + 50u), true);
    check(wrap.stableClosed() == true, "float bench debounce across uint32 wrap");
    FloatSafetyInput safety;
    check(safety.level() == Reservoir::Unknown, "physical float boot UNKNOWN");
    check(safety.update(0, true) == Reservoir::Unknown && safety.update(49, true) == Reservoir::Unknown,
          "physical float closing does not immediately allow water present");
    check(safety.update(50, true) == Reservoir::Present, "physical float stable closed indicates present");
    check(safety.update(51, false) == Reservoir::Low, "physical float opening immediately blocks without debounce delay");
    check(safety.update(52, true) == Reservoir::Unknown && safety.update(101, true) == Reservoir::Unknown &&
          safety.update(102, true) == Reservoir::Present, "physical float bounce requires fresh closing window");
    FloatSafetyInput safetyWrap;
    safetyWrap.update(edge, true);
    check(safetyWrap.update(std::uint32_t(edge + 50u), true) == Reservoir::Present,
          "physical float closing window handles wrap");
    IrrigationController interlock;
    check(interlock.manualPulse(103, safety.level(), 100) == RequestResult::Started,
          "physical float indication reaches local interlock");
    interlock.evaluateSafety(104, safety.update(104, false));
    check(!interlock.pumpRequestedOn() && interlock.state() == IrrigationState::BlockedLowWater,
          "physical float opening cancels local pump request immediately");
}
