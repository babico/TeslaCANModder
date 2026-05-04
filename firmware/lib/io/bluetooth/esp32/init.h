#pragma once
#include <BluetoothSerial.h>
#include "core/config/esp32/board.h"

static BluetoothSerial btSerial;
static bool btReady = false;

void btInit()
{
	btSerial.begin(BLE_DEVICE_NAME);
	btReady = true;
}

bool btIsReady() { return btReady; }
