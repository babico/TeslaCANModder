#pragma once

#include "board/bridge.h"
#include "board/state.h"
#include "can/frame.h"
#include "drivers/runtime/driver.h"
#include "handlers/index.h"

static Driver *appDriver = nullptr;
static Handler *appHandler = nullptr;
static BoardBridge appBridge;
static BoardState appState;
static bool appDriverReady = false;

static volatile bool frameReady = true;
static void canISR() { frameReady = true; }

static void configureAppHandler()
{
    appHandler = board::createHandler(appState.variant());
    appState.applyTo(*appHandler);

    if (appDriverReady)
    {
        appDriver->setFilters(appHandler->filterIds(), appHandler->filterIdCount());
    }
}

template <typename Driver>
static void appSetup(Driver &drv, const char *readyMsg)
{
    delay(1000);

    Serial.begin(115200);
    while (!Serial && millis() < 2000)
    {
    }

    configureAppHandler();
    appState.setInstallReadiness(BoardState::InstallReadiness::BenchReady);
    appBridge.begin(appState);

    appDriver = &drv;
    appDriverReady = appDriver->init();
    if (!appDriverReady)
    {
        appBridge.sendLog("ERROR: CAN initialization failed. Check shield wiring, oscillator and SPI pins.");
        return;
    }

    appState.setInstallReadiness(BoardState::InstallReadiness::InstalledPowerReady);
    configureAppHandler();

    if (Driver::kSupportsISR)
    {
        appDriver->enableInterrupt(canISR);
    }

    appBridge.sendLog(readyMsg);
}

template <typename Driver>
static void appLoop()
{
    appState.syncFrom(*appHandler);
    appBridge.tick(appState);

    if (appState.consumeVariantChange())
    {
        configureAppHandler();
    }
    else
    {
        appState.applyTo(*appHandler);
    }

    if (!appDriverReady)
    {
        delay(10);
        return;
    }

    if (Driver::kSupportsISR)
    {
        if (!frameReady)
        {
            return;
        }
        frameReady = false;
    }

    Frame frame;
    while (appDriver->read(frame))
    {
        // Live CAN traffic is the cheapest signal that the install moved past
        // bench bring-up and into a usable runtime state.
        appState.setInstallReadiness(BoardState::InstallReadiness::RuntimeReady);
        digitalWrite(PIN_LED, LOW);
        appBridge.sendFrame(frame, "rx");
        appHandler->handleMessage(frame, *appDriver);
    }
    digitalWrite(PIN_LED, HIGH);
}
