#include "WaterRuntime.hpp"
namespace water {
WaterRuntime::WaterRuntime(IClock& clock, IFloatSwitch& reservoir, ISoilSensor& soil, IPumpDriver& pump,
                           std::optional<AutoConfig> config, LabLimits limits)
    : clock_(clock), reservoir_(reservoir), soil_(soil), pump_(pump), controller_(config, limits) {}
void WaterRuntime::begin() {
    if (begun_) return;
    pump_.apply(false);
    begun_ = true;
}
void WaterRuntime::tick() {
    begin();
    // Apply safety before acquiring soil; adapters must still be nonblocking.
    const auto reservoir = reservoir_.read();
    controller_.evaluateSafety(clock_.nowMs(), reservoir);
    pump_.apply(controller_.pumpRequestedOn());
    if (controller_.mode() == IrrigationMode::Auto) {
        const auto reading = soil_.read();
        const auto currentReservoir = reservoir_.read();
        controller_.update(clock_.nowMs(), currentReservoir, reading);
        pump_.apply(controller_.pumpRequestedOn());
    }
}
bool WaterRuntime::setMode(IrrigationMode mode) {
    begin();
    const bool changed = controller_.setMode(clock_.nowMs(), mode);
    const auto reservoir = reservoir_.read();
    controller_.evaluateSafety(clock_.nowMs(), reservoir);
    pump_.apply(controller_.pumpRequestedOn());
    return changed;
}
RequestResult WaterRuntime::manualPulse(std::uint32_t durationMs) {
    begin();
    const auto reservoir = reservoir_.read();
    const auto result = controller_.manualPulse(clock_.nowMs(), reservoir, durationMs);
    pump_.apply(controller_.pumpRequestedOn());
    return result;
}
}
