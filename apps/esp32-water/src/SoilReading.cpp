#include "SoilReading.hpp"
#include <algorithm>
#include <cmath>
namespace water {
SoilReading SoilReading::invalid(Origin origin, std::uint32_t at) { return {origin, at}; }
SoilReading SoilReading::physical(std::optional<int> raw, std::uint32_t at,
                                  std::optional<SoilCalibration> calibration) {
    SoilReading result(Origin::PhysicalSensor, at);
    result.raw_ = raw;
    if (!raw || !calibration) return result;
    const auto& c = *calibration;
    if (c.allowedRawMin >= c.allowedRawMax || c.dryRaw == c.wetRaw ||
        c.dryRaw < c.allowedRawMin || c.dryRaw > c.allowedRawMax ||
        c.wetRaw < c.allowedRawMin || c.wetRaw > c.allowedRawMax ||
        *raw < std::min(c.dryRaw, c.wetRaw) || *raw > std::max(c.dryRaw, c.wetRaw)) return result;
    result.pct_ = 100.0 * (double(*raw) - double(c.dryRaw)) / (double(c.wetRaw) - double(c.dryRaw));
    return result;
}
SoilReading SoilReading::simulated(double pct, std::uint32_t at) {
    SoilReading result(Origin::SimulatedInput, at);
    if (std::isfinite(pct) && pct >= 0 && pct <= 100) result.pct_ = pct;
    return result;
}
Quality SoilReading::quality(std::uint32_t now, std::uint32_t maxAgeMs) const {
    if (!pct_ || maxAgeMs == 0 || maxAgeMs >= 0x80000000u) return Quality::Invalid;
    return std::uint32_t(now - observedAt_) > maxAgeMs ? Quality::Stale : Quality::Valid;
}
}
