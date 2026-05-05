#pragma once
#include "client/common/api_fwd.h"

// ── API Key Generation & Validation ─────────────────────────────────────────
static Preferences authPrefs;
#define AUTH_NVS_NS "tcm_auth"

static void generateApiKey(char *key, size_t len)
{
	static const char hex[] = "0123456789abcdef";
	for (size_t i = 0; i < len - 1; i++)
	{
		key[i] = hex[esp_random() % 16];
	}
	key[len - 1] = '\0';
}

static void loadOrCreateApiKey(State &s)
{
	authPrefs.begin(AUTH_NVS_NS, true);
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
		authPrefs.begin(AUTH_NVS_NS, false);
		authPrefs.putString("key", s.apiKey);
		authPrefs.putBool("required", s.apiKeyRequired);
		authPrefs.end();
		char msg[80];
		snprintf(msg, sizeof(msg), "API key generated: %.8s...", s.apiKey);
		sendLog(msg);
	}
}

static bool requireAuth()
{
	if (!restState || !restState->apiKeyRequired)
		return true;
	String provided = server.header("X-API-Key");
	if (provided.length() == 0)
		provided = server.arg("apiKey");
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
