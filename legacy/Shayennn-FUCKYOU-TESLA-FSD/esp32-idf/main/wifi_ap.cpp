#include "wifi_ap.h"

#include <cstring>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "lwip/ip_addr.h"

namespace {

constexpr char kLogTag[] = "wifi_ap";
constexpr char kSsid[] = "TeslaFSD";
constexpr uint8_t kChannel = 6;
constexpr uint8_t kMaxConn = 4;

}

namespace wifi_ap {

void start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_netif_t* ap_netif = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_cfg = {};
    std::memcpy(wifi_cfg.ap.ssid, kSsid, sizeof(kSsid));
    wifi_cfg.ap.ssid_len = sizeof(kSsid) - 1;
    wifi_cfg.ap.channel = kChannel;
    wifi_cfg.ap.max_connection = kMaxConn;
    wifi_cfg.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_dhcps_stop(ap_netif);

    uint8_t offer_router = 0;
    uint8_t offer_dns = 0;
    ESP_ERROR_CHECK(esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET,
                                           ESP_NETIF_ROUTER_SOLICITATION_ADDRESS,
                                           &offer_router, sizeof(offer_router)));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET,
                                           ESP_NETIF_DOMAIN_NAME_SERVER,
                                           &offer_dns, sizeof(offer_dns)));

    esp_netif_ip_info_t ip_info = {};
    IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
    IP4_ADDR(&ip_info.gw, 0, 0, 0, 0);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));

    esp_netif_dns_info_t dns = {};
    IP4_ADDR(&dns.ip.u_addr.ip4, 0, 0, 0, 0);
    dns.ip.type = ESP_IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns));

    esp_netif_dhcps_start(ap_netif);

    ESP_LOGI(kLogTag, "AP started: SSID=%s CH=%d (no gateway, no DNS)", kSsid, kChannel);
}

}
