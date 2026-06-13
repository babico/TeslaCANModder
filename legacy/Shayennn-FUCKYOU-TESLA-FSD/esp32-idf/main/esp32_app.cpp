#include <cstring>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_twai_onchip.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "auth.h"
#include "can_fast_path.h"
#include "can_logger.h"
#include "wifi_ap.h"
#include "web_server.h"
#include "ota.h"

namespace {

constexpr char kLogTag[] = "tesla_fsd";
constexpr gpio_num_t kCanRxPin = GPIO_NUM_27;
constexpr gpio_num_t kCanTxPin = GPIO_NUM_26;

twai_node_handle_t gNode = nullptr;

}

extern "C" void app_main() {
    ESP_LOGI(kLogTag, "Tesla FSD CAN Mod starting...");
    ESP_LOGI(kLogTag, "Reset reason: %d", static_cast<int>(esp_reset_reason()));

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    can_logger::init();

    twai_onchip_node_config_t node_cfg = {};
    node_cfg.io_cfg.tx = kCanTxPin;
    node_cfg.io_cfg.rx = kCanRxPin;
    node_cfg.io_cfg.quanta_clk_out = GPIO_NUM_NC;
    node_cfg.io_cfg.bus_off_indicator = GPIO_NUM_NC;
    node_cfg.bit_timing.bitrate = 500000;
    node_cfg.tx_queue_depth = 1;
    node_cfg.intr_priority = 1;
    ESP_ERROR_CHECK(twai_new_node_onchip(&node_cfg, &gNode));

    can_fast::init(gNode);
    can_fast::set_recovery_callback(&auth::clear_password);
    can_fast::start();

    wifi_ap::start();
    ota_update::init();
    web_server_start();

    ESP_LOGI(kLogTag, "System ready");
}
