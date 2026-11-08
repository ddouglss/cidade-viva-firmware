#include "EnergySafety.hpp"
namespace energy {
OutputIntent bootIntent() { return {}; }
bool hasConflictingGreens(const OutputIntent& intent) {
    return intent.trafficA == Signal::Green && intent.trafficB == Signal::Green;
}
}
