#include "can_logger.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

namespace {

constexpr uint32_t kMask = can_logger::kLogCapacity - 1;

can_logger::Entry gBuf[can_logger::kLogCapacity];
uint32_t gHead = 0;
uint32_t gTail = 0;
uint32_t gLost = 0;
portMUX_TYPE gLogMux = portMUX_INITIALIZER_UNLOCKED;

}

namespace can_logger {

void init() {
    gHead = 0;
    gTail = 0;
    gLost = 0;
}

void push(const tesla_fsd::can_frame& frame, Dir dir) {
    taskENTER_CRITICAL(&gLogMux);
    uint32_t next = (gHead + 1) & kMask;
    if (next == gTail) {
        gTail = (gTail + 1) & kMask;
        gLost++;
    }
    Entry& e = gBuf[gHead];
    e.frame = frame;
    e.timestamp_us = (uint32_t)esp_timer_get_time();
    e.dir = dir;
    gHead = next;
    taskEXIT_CRITICAL(&gLogMux);
}

size_t snapshot(Entry* out, size_t max_entries, size_t* lost) {
    taskENTER_CRITICAL(&gLogMux);
    *lost = gLost;
    uint32_t tail = gTail;
    uint32_t head = gHead;
    size_t avail = (head - tail) & kMask;
    if (avail > max_entries) {
        tail = (head - max_entries) & kMask;
        avail = max_entries;
    }
    for (size_t i = 0; i < avail; ++i) {
        out[i] = gBuf[(tail + i) & kMask];
    }
    taskEXIT_CRITICAL(&gLogMux);
    return avail;
}

size_t count() {
    taskENTER_CRITICAL(&gLogMux);
    size_t avail = (gHead - gTail) & kMask;
    taskEXIT_CRITICAL(&gLogMux);
    return avail;
}

}
