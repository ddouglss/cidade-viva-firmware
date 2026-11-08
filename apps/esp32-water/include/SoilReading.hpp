#pragma once
#include <cstdint>
#include <optional>
namespace water {
enum class Origin { PhysicalSensor, SimulatedInput };
enum class Quality { Valid, Invalid, Stale };
struct SoilCalibration {
    int dryRaw;
    int wetRaw;
    int allowedRawMin;
    int allowedRawMax;
};
class SoilReading {
public:
    static SoilReading physical(std::optional<int> raw, std::uint32_t observedAt,
                                std::optional<SoilCalibration> calibration = std::nullopt);
    static SoilReading simulated(double relativePct, std::uint32_t observedAt);
    static SoilReading invalid(Origin origin, std::uint32_t observedAt);
    Quality quality(std::uint32_t now, std::uint32_t maxAgeMs) const;
    Origin origin() const { return origin_; }
    std::optional<int> raw() const { return raw_; }
    std::optional<double> relativePct() const { return pct_; }
    std::uint32_t observedAt() const { return observedAt_; }
private:
    SoilReading(Origin origin, std::uint32_t at) : origin_(origin), observedAt_(at) {}
    Origin origin_;
    std::uint32_t observedAt_;
    std::optional<int> raw_;
    std::optional<double> pct_;
};
}
