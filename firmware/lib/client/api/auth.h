#pragma once

/**
 * @file firmware/lib/client/api/auth.h
 * @brief API key generation, persistence, and request authentication for the REST API.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/common/api_fwd.h"

static Preferences authPrefs;
#define AUTH_NVS_NS "tcm_auth"

/**
 * @brief Generate a random hex API key string.
 * @param key Output buffer to receive the null-terminated hex key.
 * @param len Size of the output buffer (key will be len-1 hex chars + null terminator).
 */
static void generateApiKey(char *key, size_t len)
{
	static const char hex[] = "0123456789abcdef";
	for (size_t i = 0; i < len - 1; i++)
	{
		key[i] = hex[esp_random() % 16]; // Hardware RNG provides entropy
	}
	key[len - 1] = '\0';
}

/**
 * @brief Load an existing API key from NVS or generate and persist a new one.
 * @param s Global state struct; apiKey and apiKeyRequired fields are populated.
 */
static void loadOrCreateApiKey(State &s)
{
	authPrefs.begin(AUTH_NVS_NS, true); // Read-only open
	String saved = authPrefs.getString("key", "");
	s.apiKeyRequired = authPrefs.getBool("required", false);
	authPrefs.end();

	if (saved.length() == 32)
	{
		strncpy(s.apiKey, saved.c_str(), sizeof(s.apiKey) - 1);
		s.apiKey[sizeof(s.apiKey) - 1] = '\0';
	}
	else
	{
		generateApiKey(s.apiKey, sizeof(s.apiKey));
		authPrefs.begin(AUTH_NVS_NS, false); // Read-write open to persist new key
		authPrefs.putString("key", s.apiKey);
		authPrefs.putBool("required", s.apiKeyRequired);
		authPrefs.end();
		char msg[80];
		snprintf(msg, sizeof(msg), "API key generated: %.8s...", s.apiKey);
		sendLog(msg);
	}
}

/**
 * @brief Validate the incoming request against the stored API key.
 *
 * Checks the X-API-Key header first, then falls back to the apiKey query parameter.
 * If authentication is not required (apiKeyRequired == false), always returns true.
 *
 * @return true if the request is authorized or auth is disabled; false if rejected
 *         (a 401/403 JSON response is sent automatically on failure).
 */
static bool requireAuth()
{
	if (!restState || !restState->apiKeyRequired)
		return true; // Auth disabled — allow all requests

	String provided = server.header("X-API-Key");
	if (provided.length() == 0)
		provided = server.arg("apiKey"); // Fallback to query parameter

	if (provided.length() == 0)
	{
		sendJsonResponse(401, "{\"error\":\"missing X-API-Key header\"}");
		return false;
	}
	if (strncmp(provided.c_str(), restState->apiKey, 32) != 0)
	{
		sendJsonResponse(403, "{\"error\":\"invalid API key\"}");
		return false;
	}
	return true;
}
