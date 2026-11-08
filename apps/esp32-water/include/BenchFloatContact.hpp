#pragma once
#include <cstdint>
#include <optional>

namespace water {
class BenchFloatContact {
public:
    void update(std::uint32_t now, bool closedToGround) {
        if (!initialized_ || candidate_ != closedToGround) {
            initialized_ = true;
            candidate_ = closedToGround;
            changedAt_ = now;
            stable_.reset();
        } else if (std::uint32_t(now - changedAt_) >= 50u) {
            stable_ = candidate_;
        }
    }
    // No domain Reservoir conversion: orientation/fail-safe wiring unvalidated.
    std::optional<bool> stableClosed() const { return stable_; }
private:
    bool initialized_ = false;
    bool candidate_ = false;
    std::uint32_t changedAt_ = 0;
    std::optional<bool> stable_;
};
}
