#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "board/config.h"
#include "drivers/mcp2515/arduino_mcp2515.h"
#include "board/app.h"

// The firmware keeps one physical CAN driver instance alive for the whole
// device lifetime. Runtime vehicle changes happen above this layer by swapping
// handlers, not by rebuilding the driver.
static ArduinoMCP2515 appDriverInstance;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    // appSetup() owns serial bring-up, board-state bootstrapping, handler
    // creation, and CAN-driver initialization before the main loop starts.
    appSetup<ArduinoMCP2515>(
        appDriverInstance,
        board::kArduinoReadyMessage
    );
}

void loop() {
    // appLoop() keeps the command bridge, status reporting, and CAN dispatch
    // in one place so the sketch entry point stays intentionally thin.
    appLoop<ArduinoMCP2515>();
}
