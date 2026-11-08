#include "EnergyRuntime.hpp"
namespace energy {
EnergyRuntime::EnergyRuntime(IClock& clock, IEnergyHardware& hardware,
                             std::optional<LightConfig> lightConfig,
                             std::optional<TrafficConfig> trafficConfig, std::uint32_t debounceMs)
    : clock_(clock), hardware_(hardware), light_(lightConfig), traffic_(trafficConfig), button_(debounceMs) {}
void EnergyRuntime::begin() {
    if (begun_) return;
    hardware_.apply(bootIntent());
    begun_ = true;
}
void EnergyRuntime::apply() {
    auto intent = traffic_.intent();
    intent.mainLightOn = light_.requestedOn();
    hardware_.apply(intent);
}
void EnergyRuntime::tick() {
    begin();
    const bool pressed = hardware_.crossingPressed();
    const auto now = clock_.nowMs();
    if (button_.update(now, pressed)) traffic_.requestCrossing();
    traffic_.update(now);
    apply();
    if (light_.mode() == LightMode::Auto) {
        const auto reading = hardware_.readLux();
        light_.update(clock_.nowMs(), reading);
        apply();
    }
}
bool EnergyRuntime::setLightMode(LightMode mode) {
    begin();
    const bool accepted = light_.setMode(mode);
    apply();
    return accepted;
}
bool EnergyRuntime::setManualLight(bool on) {
    begin();
    const bool accepted = light_.setManual(on);
    apply();
    return accepted;
}
}
