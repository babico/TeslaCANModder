#pragma once

#include <cstddef>
#include <cstdint>

namespace auth {

constexpr size_t kChallengeLen = 16;
constexpr size_t kHashLen = 32;
constexpr size_t kPasswordMaxLen = 63;

struct ChallengeResponse {
    uint8_t challenge[kChallengeLen];
};

bool has_password();
void set_password(const char* password);
void clear_password();
bool verify_response(const uint8_t* response, size_t len);
bool verify_session_token(const uint8_t* token, size_t len);
ChallengeResponse get_challenge();

}
