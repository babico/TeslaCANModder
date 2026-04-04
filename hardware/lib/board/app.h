#pragma once

#include "board/bridge.h"
#include "board/state.h"
#include "can/frame.h"
#include "drivers/runtime/driver.h"
#include "handlers/index.h"

static Driver *appDriver = nullptr;
static BoardBridge appBridge;
static BoardState appState;
static Driver *appHardwareDriver = nullptr;
static Handler *appHandler = nullptr;
static bool appDriverReady = false;

static volatile bool frameReady = true;
static void canISR() { frameReady = true; }

class BridgeDriver final : public Driver
{
public:
    bool init() override
    {
        return appHardwareDriver != nullptr && appHardwareDriver->init();
    }

    void setFilters(const uint32_t *ids, uint8_t count) override
    {
        if (appHardwareDriver != nullptr)
        {
            appHardwareDriver->setFilters(ids, count);
        }
    }

    bool enableInterrupt(void (*onReady)()) override
    {
        return appHardwareDriver != nullptr && appHardwareDriver->enableInterrupt(onReady);
    }

    bool read(Frame &frame) override
    {
        return appHardwareDriver != nullptr && appHardwareDriver->read(frame);
    }

    void send(const Frame &frame) override
    {
        if (appHardwareDriver == nullptr)
        {
            return;
        }

        appHardwareDriver->send(frame);
        appBridge.sendFrame(frame, "tx", millis());
    }
};

static BridgeDriver appBridgeDriver;
static Driver *appObservedDriver = &appBridgeDriver;

static void configureAppHandler()
{
    // Vehicle behavior is runtime-selectable, so the board recreates the
    // active handler whenever the variant changes and reapplies the shared
    // board state to the new handler before CAN traffic resumes.
    appHandler = board::createHandler(appState.variant());
    appState.applyTo(*appHandler);

    if (appDriverReady)
    {
        if (appState.rawCanListen())
        {
            appObservedDriver->setFilters(nullptr, 0);
        }
        else
        {
            appObservedDriver->setFilters(appHandler->filterIds(), appHandler->filterIdCount());
        }
    }
}

template <typename Driver>
static void appSetup(Driver &drv, const char *readyMsg)
{
    // Give the USB serial bridge a brief moment to settle after reset so the
    // first boot/status messages are less likely to be lost on host attach.
    delay(1000);

    Serial.begin(115200);
    while (!Serial && millis() < 2000)
    {
    }

    configureAppHandler();
    appState.setInstallReadiness(BoardState::InstallReadiness::BenchReady);
    appBridge.begin(appState);

    appHardwareDriver = &drv;
    appDriver = appObservedDriver;
    appDriverReady = appObservedDriver->init();
    if (!appDriverReady)
    {
        appBridge.sendLog("ERROR: CAN initialization failed. Check shield wiring, oscillator and SPI pins.");
        return;
    }

    // A successful CAN-driver init means the install is electrically ready to
    // move beyond the bench phase, even if the board has not seen live traffic
    // yet.
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
    // Keep the bridge and handler state synchronized on every pass so status
    // replies, command effects, and frame handling all read the same board
    // state.
    appState.syncFrom(*appHandler);
    appBridge.tick(appState);

    if (appState.consumeVariantChange())
    {
        configureAppHandler();
    }
    else if (appState.consumeRawCanListenChange())
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
        appBridge.sendFrame(frame, "rx", millis());
        appHandler->handleMessage(frame, *appDriver);
    }
    digitalWrite(PIN_LED, HIGH);
}
