#include "ota.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr char kLogTag[] = "ota";

esp_ota_handle_t gHandle = 0;
const esp_partition_t* gPartition = nullptr;
bool gInProgress = false;
esp_timer_handle_t gRebootTimer = nullptr;

void rebootTimerCb(void*) {
    esp_restart();
}

}

namespace ota_update {

void init() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    ESP_LOGI(kLogTag, "Running partition: %s offset=0x%x",
             running->label, running->address);

    if (!gRebootTimer) {
        esp_timer_create_args_t timer_args = {};
        timer_args.callback = &rebootTimerCb;
        timer_args.name = "ota_reboot";
        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &gRebootTimer));
    }
}

bool begin() {
    if (gInProgress) return false;
    gPartition = esp_ota_get_next_update_partition(nullptr);
    if (!gPartition) {
        ESP_LOGE(kLogTag, "No OTA partition found");
        return false;
    }
    esp_err_t err = esp_ota_begin(gPartition, OTA_WITH_SEQUENTIAL_WRITES, &gHandle);
    if (err != ESP_OK) {
        ESP_LOGE(kLogTag, "esp_ota_begin failed: %s", esp_err_to_name(err));
        return false;
    }
    gInProgress = true;
    ESP_LOGI(kLogTag, "OTA begin: target=%s offset=0x%x", gPartition->label, gPartition->address);
    return true;
}

bool write(const uint8_t* data, size_t len) {
    if (!gInProgress) return false;
    esp_err_t err = esp_ota_write(gHandle, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(kLogTag, "esp_ota_write failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool end() {
    if (!gInProgress) return false;
    esp_err_t err = esp_ota_end(gHandle);
    gInProgress = false;
    if (err != ESP_OK) {
        ESP_LOGE(kLogTag, "esp_ota_end failed: %s", esp_err_to_name(err));
        return false;
    }
    err = esp_ota_set_boot_partition(gPartition);
    if (err != ESP_OK) {
        ESP_LOGE(kLogTag, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(kLogTag, "OTA complete, reboot pending");
    return true;
}

void schedule_reboot(uint32_t delay_ms) {
    if (!gRebootTimer) return;
    esp_timer_stop(gRebootTimer);
    ESP_ERROR_CHECK(esp_timer_start_once(gRebootTimer, static_cast<uint64_t>(delay_ms) * 1000ULL));
}

void abort() {
    if (!gInProgress) return;
    esp_ota_abort(gHandle);
    gInProgress = false;
    ESP_LOGW(kLogTag, "OTA aborted");
}

bool in_progress() { return gInProgress; }

}
