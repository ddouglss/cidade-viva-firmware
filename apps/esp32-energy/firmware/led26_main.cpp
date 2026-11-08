#include <Arduino.h>
#include "BenchLedBlink.hpp"

namespace {
// Isolated bench LED only; not a final assignment of MAIN_LIGHT or traffic pins.
constexpr int benchLedGpio = 26;
energy::BenchLedBlink blink(0);

void report(std::uint32_t now) {
    Serial.printf("LED26_REQUEST uptimeMs=%lu gpio=26 level=%s physicalFeedback=NONE\n",
                  static_cast<unsigned long>(now), blink.requestedHigh() ? "HIGH" : "LOW");
}
}

void setup() {
    digitalWrite(benchLedGpio, LOW);
    pinMode(benchLedGpio, OUTPUT);
    blink = energy::BenchLedBlink(millis());
    Serial.begin(115200);
    Serial.println();
    Serial.println("CIDADE_VIVA_ENERGY_LED26_TEST v1");
    Serial.println("deviceId=esp32-energy externalOutputs=BENCH_LED26_ONLY");
    Serial.println("BENCH_ONLY: red LED with series 220 ohm resistor to GND; no other application GPIO.");
    Serial.println("No WiFi/MQTT or sensors; GPIO request is not measured light.");
    report(millis());
}

void loop() {
    const auto now = static_cast<std::uint32_t>(millis());
    if (blink.update(now)) {
        digitalWrite(benchLedGpio, blink.requestedHigh() ? HIGH : LOW);
        report(now);
    }
    delay(1);
}
