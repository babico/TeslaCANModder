#include "auth.h"

#include <cstring>
#include "esp_log.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include "sha/sha_core.h"

namespace {

constexpr char kLogTag[] = "auth";
constexpr char kNvsNamespace[] = "auth";
constexpr char kNvsKey[] = "pw_hash";

uint8_t gCurrentChallenge[auth::kChallengeLen] = {};
uint8_t gSessionToken[auth::kHashLen] = {};
bool gSessionValid = false;

void sha256(const void* input, size_t len, uint8_t* out) {
    esp_sha(SHA2_256,
            reinterpret_cast<const unsigned char*>(input),
            len,
            out);
}

bool constantTimeEq(const uint8_t* a, const uint8_t* b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; ++i) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

}

namespace auth {

bool has_password() {
    nvs_handle_t h;
    if (nvs_open(kNvsNamespace, NVS_READONLY, &h) != ESP_OK) return false;
    size_t len = 0;
    esp_err_t err = nvs_get_blob(h, kNvsKey, nullptr, &len);
    nvs_close(h);
    return err == ESP_OK && len == kHashLen;
}

void set_password(const char* password) {
    uint8_t hash[kHashLen];
    sha256(password, std::strlen(password), hash);

    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open(kNvsNamespace, NVS_READWRITE, &h));
    ESP_ERROR_CHECK(nvs_set_blob(h, kNvsKey, hash, kHashLen));
    ESP_ERROR_CHECK(nvs_commit(h));
    nvs_close(h);
    gSessionValid = false;
    ESP_LOGI(kLogTag, "Password set");
}

void clear_password() {
    nvs_handle_t h;
    if (nvs_open(kNvsNamespace, NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_key(h, kNvsKey);
        nvs_commit(h);
        nvs_close(h);
    }
    std::memset(gSessionToken, 0, sizeof(gSessionToken));
    std::memset(gCurrentChallenge, 0, sizeof(gCurrentChallenge));
    gSessionValid = false;
    ESP_LOGW(kLogTag, "Password cleared via recovery flow");
}

ChallengeResponse get_challenge() {
    ChallengeResponse cr;
    esp_fill_random(cr.challenge, kChallengeLen);
    std::memcpy(gCurrentChallenge, cr.challenge, kChallengeLen);
    return cr;
}

bool verify_response(const uint8_t* response, size_t len) {
    if (len != kHashLen) return false;

    nvs_handle_t h;
    if (nvs_open(kNvsNamespace, NVS_READONLY, &h) != ESP_OK) return false;

    uint8_t stored[kHashLen];
    size_t stored_len = kHashLen;
    esp_err_t err = nvs_get_blob(h, kNvsKey, stored, &stored_len);
    nvs_close(h);
    if (err != ESP_OK || stored_len != kHashLen) return false;

    uint8_t expected[kHashLen];
    uint8_t combined[kHashLen + kChallengeLen];
    std::memcpy(combined, stored, kHashLen);
    std::memcpy(combined + kHashLen, gCurrentChallenge, kChallengeLen);
    sha256(combined, sizeof(combined), expected);

    esp_fill_random(gCurrentChallenge, kChallengeLen);

    bool ok = constantTimeEq(response, expected, kHashLen);
    if (ok) {
        std::memcpy(gSessionToken, response, kHashLen);
        gSessionValid = true;
    }
    return ok;
}

bool verify_session_token(const uint8_t* token, size_t len) {
    if (!gSessionValid || len != kHashLen) return false;
    return constantTimeEq(token, gSessionToken, kHashLen);
}

}
