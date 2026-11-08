#pragma once
#include <cstdint>

namespace energy {
// Diagnostic timing only, unrelated to traffic/light product configuration.
class BenchLedBlink {
public:
    explicit BenchLedBlink(std::uint32_t startedAt) : changedAt_(startedAt) {}
    bool update(std::uint32_t now) {
        if (std::uint32_t(now - changedAt_) >= 500u) {
            changedAt_ = now;
            high_ = !high_;
            return true;
        }
        return false;
    }
    bool requestedHigh() const { return high_; }
private:
    std::uint32_t changedAt_;
    bool high_ = false;
};
}
