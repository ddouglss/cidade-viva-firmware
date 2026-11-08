#include "EnergySafety.hpp"
#include <cstdlib>
#include <iostream>
using namespace energy;
void controllerTests();
void check(bool ok, const char* name) {
    if (!ok) { std::cerr << "FAIL: " << name << '\n'; std::exit(1); }
    std::cout << "PASS: " << name << '\n';
}
int main() {
    const auto boot = bootIntent();
    check(!boot.mainLightOn && !boot.buzzerOn && boot.trafficA == Signal::Red &&
          boot.trafficB == Signal::Red, "boot logical all red and loads OFF");
    for (auto a : {Signal::Red, Signal::Yellow, Signal::Green}) {
        for (auto b : {Signal::Red, Signal::Yellow, Signal::Green}) {
            OutputIntent intent{false, false, a, b};
            check(hasConflictingGreens(intent) == (a == Signal::Green && b == Signal::Green), "green invariant matrix");
        }
    }
    check(!hasConflictingGreens(bootIntent()), "recreated boot has no green conflict");
    controllerTests();
}
