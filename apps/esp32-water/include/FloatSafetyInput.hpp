#pragma once
#include "BenchFloatContact.hpp"
#include "PumpSafety.hpp"

namespace water {
// Orientation reported on this bench: raised=closed, lowered=open.
// An opening blocks immediately. A closing must settle before allowing Present.
class FloatSafetyInput {
public:
    Reservoir update(std::uint32_t now, bool closedToGround) {
        contact_.update(now, closedToGround);
        if (!closedToGround) level_ = Reservoir::Low;
        else level_ = contact_.stableClosed() == true ? Reservoir::Present : Reservoir::Unknown;
        return level_;
    }
    Reservoir level() const { return level_; }
private:
    BenchFloatContact contact_;
    Reservoir level_ = Reservoir::Unknown;
};
}
