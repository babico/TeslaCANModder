#pragma once

#include <cstdint>

namespace ota_update {

void init();
bool begin();
bool write(const uint8_t* data, size_t len);
bool end();
void schedule_reboot(uint32_t delay_ms);
void abort();
bool in_progress();

}
