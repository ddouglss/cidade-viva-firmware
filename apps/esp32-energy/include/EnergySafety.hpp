#pragma once

namespace energy {
enum class Signal { Red, Yellow, Green };
struct OutputIntent {
    bool mainLightOn = false;
    bool buzzerOn = false;
    Signal trafficA = Signal::Red;
    Signal trafficB = Signal::Red;
};
// Logical intent only. Physical polarity, mapping and application are pending.
OutputIntent bootIntent();
bool hasConflictingGreens(const OutputIntent& intent);
}
