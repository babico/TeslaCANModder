/*
 * TeslaCam — ESP32-S3 Camera (OV5640)
 * SoftAP + UDP raw streaming to ESP32-S3 Display
 *
 * Captures JPEG frames at VGA (640×480) and sends them
 * via raw lwip UDP to the broadcast address 192.168.4.255:5000.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "esp_camera.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

// ── WiFi SoftAP ─────────────────────────────────────────────────────────────────
static const char *AP_SSID = "TeslaCam";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_GW(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);
#define WIFI_CHANNEL 1  // WiFi channel (must match Bridge + Display)

// ── Receiver ────────────────────────────────────────────────────────────────────
static const uint16_t  RECEIVER_PORT = 5000;

// ── UDP chunking ────────────────────────────────────────────────────────────────
#define CHUNK_PAYLOAD 1400          // Conservative: well under MTU
#define HEADER_SIZE   8

// ── OV5640 pin mapping (ESP32-S3 WROOM / Freenove) ─────────────────────────────
#define PWDN_GPIO   -1
#define RESET_GPIO  -1
#define XCLK_GPIO   15
#define SIOD_GPIO    4
#define SIOC_GPIO    5
#define Y9_GPIO     16
#define Y8_GPIO     17
#define Y7_GPIO     18
#define Y6_GPIO     12
#define Y5_GPIO     10
#define Y4_GPIO      8
#define Y3_GPIO      9
#define Y2_GPIO     11
#define VSYNC_GPIO   6
#define HREF_GPIO    7
#define PCLK_GPIO   13

static int udp_sock = -1;
static struct sockaddr_in dest_addr;
static uint16_t frame_id = 0;

// ── Control channel (port 5001) ─────────────────────────────────────────────────
#define CTRL_PORT 5001
#define CTRL_TIMEOUT_MS 5000       // Stop streaming if no heartbeat for 5s
static volatile bool streamEnabled = false;    // Sleep by default, wait for STRT
static volatile unsigned long lastCtrlTime = 0;

// ─────────────────────────────────────────────────────────────────────────────────
//  Camera init
// ─────────────────────────────────────────────────────────────────────────────────
bool initCamera() {
    camera_config_t cfg = {};
    cfg.ledc_channel = LEDC_CHANNEL_0;
    cfg.ledc_timer   = LEDC_TIMER_0;
    cfg.pin_d0       = Y2_GPIO;
    cfg.pin_d1       = Y3_GPIO;
    cfg.pin_d2       = Y4_GPIO;
    cfg.pin_d3       = Y5_GPIO;
    cfg.pin_d4       = Y6_GPIO;
    cfg.pin_d5       = Y7_GPIO;
    cfg.pin_d6       = Y8_GPIO;
    cfg.pin_d7       = Y9_GPIO;
    cfg.pin_xclk     = XCLK_GPIO;
    cfg.pin_pclk     = PCLK_GPIO;
    cfg.pin_vsync    = VSYNC_GPIO;
    cfg.pin_href     = HREF_GPIO;
    cfg.pin_sccb_sda = SIOD_GPIO;
    cfg.pin_sccb_scl = SIOC_GPIO;
    cfg.pin_pwdn     = PWDN_GPIO;
    cfg.pin_reset    = RESET_GPIO;
    cfg.xclk_freq_hz = 20000000;
    cfg.pixel_format = PIXFORMAT_JPEG;
    cfg.grab_mode    = CAMERA_GRAB_LATEST;

    if (psramFound()) {
        cfg.frame_size   = FRAMESIZE_HVGA;   // 480×320 – smaller JPEG, lower latency
        cfg.jpeg_quality = 18;               // Lower quality = smaller JPEG = fewer UDP chunks
        cfg.fb_count     = 2;
        cfg.fb_location  = CAMERA_FB_IN_PSRAM;
        Serial.println("[CAM] PSRAM \u2013 HVGA, 2 buffers");
    } else {
        cfg.frame_size   = FRAMESIZE_QVGA;
        cfg.jpeg_quality = 12;
        cfg.fb_count     = 1;
        cfg.fb_location  = CAMERA_FB_IN_DRAM;
        Serial.println("[CAM] No PSRAM – QVGA, 1 buffer");
    }

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("[CAM] Init failed: 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        Serial.printf("[CAM] Sensor PID: 0x%04X\n", s->id.PID);
        s->set_vflip(s, 1);     // Flip vertical (caméra de recul)
        s->set_hmirror(s, 0);   // Pas de miroir horizontal
    }
    Serial.println("[CAM] Camera ready");
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────────
//  WiFi SoftAP
// ─────────────────────────────────────────────────────────────────────────────────
void startSoftAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
    WiFi.softAP(AP_SSID, nullptr, WIFI_CHANNEL);  // Must match Bridge + Display
    Serial.printf("[WIFI] SoftAP started – SSID: %s  IP: %s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
}

// ─────────────────────────────────────────────────────────────────────────────────
//  Init raw lwip UDP socket
// ─────────────────────────────────────────────────────────────────────────────────
void initUDP() {
    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_sock < 0) {
        Serial.println("[UDP] Socket creation failed");
        return;
    }

    // Enable broadcast
    int broadcast = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // Increase send buffer
    int sndbuf = 32768;
    setsockopt(udp_sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    // Set destination: broadcast on subnet
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(RECEIVER_PORT);
    dest_addr.sin_addr.s_addr = inet_addr("192.168.4.2");

    Serial.printf("[UDP] Socket ready, target 192.168.4.2:%d\n", RECEIVER_PORT);
}

// ─────────────────────────────────────────────────────────────────────────────────
//  Send one JPEG frame via UDP chunks
// ─────────────────────────────────────────────────────────────────────────────────
void sendFrame(camera_fb_t *fb) {
    if (udp_sock < 0) return;

    uint16_t total_chunks = (fb->len + CHUNK_PAYLOAD - 1) / CHUNK_PAYLOAD;
    uint8_t packet[HEADER_SIZE + CHUNK_PAYLOAD];
    int errors = 0;

    for (uint16_t i = 0; i < total_chunks; i++) {
        uint32_t offset = (uint32_t)i * CHUNK_PAYLOAD;
        uint16_t size = (uint16_t)min((size_t)CHUNK_PAYLOAD, (size_t)(fb->len - offset));

        // Header: frame_id(2) + chunk_id(2) + total_chunks(2) + chunk_size(2)
        memcpy(packet + 0, &frame_id,      2);
        memcpy(packet + 2, &i,             2);
        memcpy(packet + 4, &total_chunks,  2);
        memcpy(packet + 6, &size,          2);
        // Payload
        memcpy(packet + HEADER_SIZE, fb->buf + offset, size);

        int ret = sendto(udp_sock, packet, HEADER_SIZE + size, 0,
                         (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (ret < 0) {
            errors++;
            if (errors == 1) {
                Serial.printf("[UDP] sendto err: errno=%d, heap=%u\n",
                              errno, ESP.getFreeHeap());
            }
            continue;
        }

        // Pace chunks: brief yield every 3 chunks to let WiFi TX drain
        if (i % 3 == 2) vTaskDelay(pdMS_TO_TICKS(1));
    }
    // Debug: log first 3 frames and any with errors
    if (frame_id < 3 || errors > 0) {
        Serial.printf("[UDP] Frame %u: %u chunks, %u bytes, %d errors\n",
                      frame_id, total_chunks, fb->len, errors);
    }
    /* Send END-of-frame marker: frameId + chunkId=0xFFFF + totalChunks + size=0 */
    {
        uint16_t endMarker = 0xFFFF;
        uint16_t zero = 0;
        memcpy(packet + 0, &frame_id,      2);
        memcpy(packet + 2, &endMarker,     2);
        memcpy(packet + 4, &total_chunks,  2);
        memcpy(packet + 6, &zero,          2);
        sendto(udp_sock, packet, HEADER_SIZE, 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    }

    frame_id++;
}

// ─────────────────────────────────────────────────────────────────────────────────
//  Sensor sleep / wake  (OV5640 software standby via register 0x3008 bit 6)
// ─────────────────────────────────────────────────────────────────────────────────
static void setSensorSleep(bool sleep) {
    sensor_t *s = esp_camera_sensor_get();
    if (!s) return;
    s->set_reg(s, 0x3008, 0x40, sleep ? 0x40 : 0x00);
    Serial.printf("[CAM] Sensor %s\n", sleep ? "sleeping" : "awake");
}

// ─────────────────────────────────────────────────────────────────────────────────
//  Streaming task — handles both control commands and frame streaming (core 1)
//  Core 0 is kept entirely free for WiFi driver processing.
// ─────────────────────────────────────────────────────────────────────────────────
void streamTask(void *pvParameters) {
    // Create and bind control socket (non-blocking)
    int ctrl_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ctrl_sock < 0) {
        Serial.println("[CTRL] Socket creation failed");
    } else {
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(CTRL_PORT);
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(ctrl_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            Serial.println("[CTRL] Bind failed");
            close(ctrl_sock);
            ctrl_sock = -1;
        }
    }
    Serial.printf("[CTRL] Listening on port %d (non-blocking)\n", CTRL_PORT);

    // Wait for first station to connect, then do one-time ARP warm-up
    Serial.println("[STREAM] Waiting for station...");
    while (WiFi.softAPgetStationNum() == 0) {
        // Still poll ctrl while waiting
        if (ctrl_sock >= 0) {
            char cbuf[8];
            recvfrom(ctrl_sock, cbuf, sizeof(cbuf), MSG_DONTWAIT, NULL, NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    Serial.println("[STREAM] Station connected — ARP warm-up (2s)");
    vTaskDelay(pdMS_TO_TICKS(2000));
    Serial.println("[STREAM] Ready — waiting for STRT command");
    lastCtrlTime = millis();  // prevent immediate heartbeat timeout
    setSensorSleep(true);     // sensor off by default until STRT

    unsigned long frames = 0;
    unsigned long t0 = millis();
    bool wasStreaming = false;

    while (true) {
        // Poll control commands (non-blocking)
        if (ctrl_sock >= 0) {
            char cbuf[8];
            int clen = recvfrom(ctrl_sock, cbuf, sizeof(cbuf) - 1,
                                MSG_DONTWAIT, NULL, NULL);
            if (clen >= 4) {
                cbuf[clen] = '\0';
                if (memcmp(cbuf, "STRT", 4) == 0) {
                    // Only enable if a station is connected (prevents stale STRT)
                    if (WiFi.softAPgetStationNum() > 0) {
                        if (!streamEnabled) {
                            Serial.println("[CTRL] START received — streaming enabled");
                        }
                        streamEnabled = true;
                        lastCtrlTime = millis();
                    }
                } else if (memcmp(cbuf, "STOP", 4) == 0) {
                    if (streamEnabled) {
                        Serial.println("[CTRL] STOP received — streaming disabled");
                    }
                    streamEnabled = false;
                }
            }
        }

        // Wait for stream to be enabled
        if (!streamEnabled) {
            if (wasStreaming) {
                Serial.println("[STREAM] Paused — STOP received");
                wasStreaming = false;
                setSensorSleep(true);
                neopixelWrite(48, 0, 0, 0);
            }
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // Heartbeat timeout check during streaming
        if (millis() - lastCtrlTime > CTRL_TIMEOUT_MS) {
            streamEnabled = false;
            neopixelWrite(48, 0, 0, 0);
            Serial.println("[CTRL] Heartbeat timeout — streaming disabled");
            continue;
        }

        if (!wasStreaming) {
            // Don't start sending until station is truly connected
            if (WiFi.softAPgetStationNum() == 0) {
                vTaskDelay(pdMS_TO_TICKS(200));
                continue;
            }
            // Wake sensor and let it stabilise before capturing
            setSensorSleep(false);
            neopixelWrite(48, 0, 20, 0);  // Dim green
            vTaskDelay(pdMS_TO_TICKS(200));
            // Drain stale ctrl commands
            if (ctrl_sock >= 0) {
                char drain[8];
                while (recvfrom(ctrl_sock, drain, sizeof(drain), MSG_DONTWAIT, NULL, NULL) > 0) {}
            }
            Serial.printf("[STREAM] Streaming started (stations=%d)\n",
                          WiFi.softAPgetStationNum());
            wasStreaming = true;
            frames = 0;
            t0 = millis();
        }

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[STREAM] Capture fail");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        sendFrame(fb);
        esp_camera_fb_return(fb);

        frames++;
        if (frames % 60 == 0) {
            float fps = frames * 1000.0f / (millis() - t0);
            Serial.printf("[STREAM] %lu frames, %.1f fps, stations=%d\n",
                          frames, fps, WiFi.softAPgetStationNum());
        }
        taskYIELD();   // minimal yield, no vTaskDelay
    }
}

// ─────────────────────────────────────────────────────────────────────────────────
//  Setup & Loop
// ─────────────────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== TeslaCam – Camera (SoftAP + UDP) ===");

    pinMode(48, OUTPUT);
    neopixelWrite(48, 0, 0, 0);  // RGB LED off
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    if (!initCamera()) {
        Serial.println("[FATAL] Camera init failed – halting");
        while (true) delay(1000);
    }

    startSoftAP();
    WiFi.setSleep(false);  // Disable WiFi power saving for max throughput
    esp_wifi_set_max_tx_power(78);  // Max TX power (19.5 dBm) for EMI resilience
    initUDP();

    // Launch streaming + control task on core 1 (core 0 free for WiFi driver)
    xTaskCreatePinnedToCore(streamTask, "stream", 8192, NULL, 1, NULL, 1);
    Serial.println("[STREAM] Task started on core 1 (waiting for STRT command)");
}

void loop() {
    delay(1000);
}
