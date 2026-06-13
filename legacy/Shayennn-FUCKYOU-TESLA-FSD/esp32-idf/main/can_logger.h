#pragma once

#include <cstdint>
#include <cstddef>
#include "vehicle_logic.h"

namespace can_logger {

enum class Dir : uint8_t { RX = 0, TX = 1 };

struct Entry {
    tesla_fsd::can_frame frame;
    uint32_t timestamp_us;
    Dir dir;
};

constexpr size_t kLogCapacity = 512;

void init();
void push(const tesla_fsd::can_frame& frame, Dir dir);
size_t snapshot(Entry* out, size_t max_entries, size_t* lost);
size_t count();

}
