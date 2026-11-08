#include <Arduino.h>
#include "BenchFloatContact.hpp"
#include "DisabledWaterHardware.hpp"

namespace {
constexpr int soilAdcGpio = 34;
constexpr int floatContactGpio = 27;
class ArduinoClock final : public water::IClock {
public:
    std::uint32_t nowMs() override { return millis(); }
};
class RawAdcSoilSensor final : public water::ISoilSensor {
public:
    water::SoilReading read() override {
        const auto raw = static_cast<int>(analogRead(soilAdcGpio));
        return water::SoilReading::physical(raw, millis()); // No calibration => no percentage.
    }
};
ArduinoClock clockSource;
RawAdcSoilSensor soilSensor;
water::UnconnectedFloatSwitch domainFloatUnknown;
water::DisabledPumpDriver disabledPump;
water::WaterRuntime runtime(clockSource, domainFloatUnknown, soilSensor, disabledPump);
water::BenchFloatContact contact;
std::optional<bool> lastContact;
std::uint32_t lastLogAt = 0;

void reportContact(std::uint32_t now, bool rawClosed) {
    const auto stable = contact.stableClosed();
    const char* state = !stable ? "SETTLING" : (*stable ? "CLOSED" : "OPEN");
    Serial.printf("FLOAT27 uptimeMs=%lu gpio=27 raw=%s contact=%s reservoir=UNKNOWN pumpOutput=DISABLED\n",
                  static_cast<unsigned long>(now), rawClosed ? "LOW" : "HIGH", state);
    lastContact = stable;
}
}

void setup() {
    runtime.begin();
    pinMode(soilAdcGpio, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(soilAdcGpio, ADC_11db);
    pinMode(floatContactGpio, INPUT_PULLUP);
    runtime.tick();
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_WATER_SOIL34_TEST v1");
    Serial.println("deviceId=esp32-water soilInput=G34 floatInput=G27 pumpOutput=DISABLED");
    Serial.println("ADC12 RAW ONLY: calibration=NONE humidityPct=UNAVAILABLE; no WiFi/MQTT.");
    Serial.println("Contact does not yet authorize pump; reservoir=UNKNOWN.");
    const auto now = clockSource.nowMs();
    const bool closed = digitalRead(floatContactGpio) == LOW;
    contact.update(now, closed);
    reportContact(now, closed);
}

void loop() {
    runtime.tick(); // MANUAL and UNKNOWN float; no actuator GPIO exists in driver.
    const auto now = clockSource.nowMs();
    const bool closed = digitalRead(floatContactGpio) == LOW;
    contact.update(now, closed);
    if (contact.stableClosed() != lastContact) reportContact(now, closed);
    if (std::uint32_t(now - lastLogAt) >= 1000u) {
        lastLogAt = now;
        const auto reading = soilSensor.read();
        Serial.printf("SOIL34 uptimeMs=%lu gpio=34 raw=%d unit=ADC_COUNT origin=PHYSICAL_SENSOR "
                      "calibration=NONE humidityPct=UNAVAILABLE pumpOutput=DISABLED\n",
                      static_cast<unsigned long>(reading.observedAt()), *reading.raw());
        reportContact(now, closed);
    }
    delay(1);
}
