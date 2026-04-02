#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "board/config.h"
#include "drivers/mcp2515/arduino_mcp2515.h"
#include "board/app.h"

static ArduinoMCP2515 appDriverInstance;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    appSetup<ArduinoMCP2515>(
        appDriverInstance,
        board::kArduinoReadyMessage
    );
}

void loop() {
    appLoop<ArduinoMCP2515>();
}
