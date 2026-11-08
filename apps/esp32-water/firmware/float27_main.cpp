#include <Arduino.h>
#include "BenchFloatContact.hpp"
#include "DisabledWaterHardware.hpp"

namespace {
constexpr int floatContactGpio = 27;
class ArduinoClock final : public water::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};
ArduinoClock clockSource;
water::UnconnectedFloatSwitch domainFloatUnknown;
water::UnconnectedSoilSensor absentSoil;
water::DisabledPumpDriver disabledPump;
water::WaterRuntime runtime(clockSource, domainFloatUnknown, absentSoil, disabledPump);
water::BenchFloatContact contact;
std::optional<bool> lastReported;
std::uint32_t lastLogAt = 0;

void report(std::uint32_t now, bool rawClosed) {
    const auto stable = contact.stableClosed();
    const char* state = !stable ? "SETTLING" : (*stable ? "CLOSED" : "OPEN");
    Serial.printf("FLOAT27 uptimeMs=%lu gpio=27 raw=%s contact=%s reservoir=UNKNOWN pumpOutput=DISABLED\n",
                  static_cast<unsigned long>(now), rawClosed ? "LOW" : "HIGH", state);
    lastReported = stable;
    lastLogAt = now;
}
}

void setup() {
    runtime.begin();
    pinMode(floatContactGpio, INPUT_PULLUP);
    runtime.tick();
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_WATER_FLOAT27_TEST v1");
    Serial.println("deviceId=esp32-water input=G27_TO_GND pumpOutput=DISABLED");
    Serial.println("OPEN/CLOSED is electrical contact only; reservoir level/orientation UNKNOWN.");
    Serial.println("No pump GPIO, WiFi/MQTT or soil sensor configured.");
    const auto now = clockSource.nowMs();
    const bool closed = digitalRead(floatContactGpio) == LOW;
    contact.update(now, closed);
    report(now, closed);
}

void loop() {
    runtime.tick();
    const auto now = clockSource.nowMs();
    const bool closed = digitalRead(floatContactGpio) == LOW;
    contact.update(now, closed);
    if (contact.stableClosed() != lastReported || std::uint32_t(now - lastLogAt) >= 1000u)
        report(now, closed);
    delay(1);
}
