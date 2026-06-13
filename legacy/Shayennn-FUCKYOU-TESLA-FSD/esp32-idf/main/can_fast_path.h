#pragma once

#include <cstdint>
#include "esp_twai.h"
#include "vehicle_logic.h"

namespace can_fast {

struct Stats {
    uint32_t rx_count = 0;
    uint32_t tx_count = 0;
    uint32_t tx_fail_count = 0;
    uint32_t filtered_count = 0;
    uint32_t consecutive_tx_fail = 0;
    bool fail_safe = false;
    bool recovery_armed = false;
    int32_t last_latency_us = 0;
    int32_t max_latency_us = 0;
    int32_t min_latency_us = 0x7FFFFFFF;
};

void init(twai_node_handle_t node);
void start();
Stats get_stats();
int get_handler_type();
void set_handler_type(int type);

using RecoveryFn = void (*)();
void set_recovery_callback(RecoveryFn fn);
void arm_recovery();

}
