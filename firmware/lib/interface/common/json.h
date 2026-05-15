#pragma once

/**
 * @file firmware/lib/interface/common/json.h
 * @brief Shared serial JSON message helpers and RPC command parser
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

// IntelliSense stubs — only visible to the IDE, not the compiler.
// Real implementations come from output.h which includes this file after defining them.
#ifdef __INTELLISENSE__
#include <Arduino.h>
#include "core/types.h"
#include "vehicle/can/ids.h"
void printStr(const char *);
void printStr(const __FlashStringHelper *);
void printNum(long);
void printHex(uint8_t);
void printLn();
#endif

#ifndef SERIAL_CMD_BUFFER_SIZE
#define SERIAL_CMD_BUFFER_SIZE 32
#endif

/**
 * @brief Streaming JSON line builder that writes key-value pairs directly to serial output
 *
 * Constructs a single-line JSON object by emitting characters as each field is added,
 * avoiding heap allocation. Supports nested objects via JsonObjectBuilder.
 */
class JsonLineBuilder
{
  public:
	JsonLineBuilder() : first_(true)
	{
		printStr(F("{"));
	}

	/**
	 * @brief Nested object builder used for sub-objects within a JSON line
	 */
	class JsonObjectBuilder
	{
	  public:
		JsonObjectBuilder() : first_(true) {}

		/**
		 * @brief Write a string key-value pair
		 * @param key JSON key name
		 * @param value Null-terminated string value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &str(const char *key, const char *value)
		{
			writeKey(key);
			printStr(F("\""));
			printStr(value);
			printStr(F("\""));
			return *this;
		}

		/**
		 * @brief Write a string key-value pair with flash-stored value
		 * @param key JSON key name
		 * @param value Flash string pointer
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &str(const char *key, const __FlashStringHelper *value)
		{
			writeKey(key);
			printStr(F("\""));
			printStr(value);
			printStr(F("\""));
			return *this;
		}

		/**
		 * @brief Write a numeric key-value pair
		 * @param key JSON key name
		 * @param value Long integer value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &num(const char *key, long value)
		{
			writeKey(key);
			printNum(value);
			return *this;
		}

		/**
		 * @brief Write a numeric key-value pair (int overload)
		 * @param key JSON key name
		 * @param value Integer value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &num(const char *key, int value)
		{
			return num(key, (long)value);
		}

		/**
		 * @brief Write a numeric key-value pair (unsigned int overload)
		 * @param key JSON key name
		 * @param value Unsigned integer value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &num(const char *key, unsigned int value)
		{
			return num(key, (long)value);
		}

		/**
		 * @brief Write a numeric key-value pair (unsigned long overload)
		 * @param key JSON key name
		 * @param value Unsigned long value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &num(const char *key, unsigned long value)
		{
			writeKey(key);
			printNum((long)value);
			return *this;
		}

		/**
		 * @brief Write a boolean key-value pair as 1 or 0
		 * @param key JSON key name
		 * @param value Boolean value
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &boolean(const char *key, bool value)
		{
			writeKey(key);
			printNum(value ? 1 : 0);
			return *this;
		}

		/**
		 * @brief Write a hex-encoded byte array as a quoted string value
		 * @param key JSON key name
		 * @param data Pointer to byte array
		 * @param len Number of bytes to encode
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &hex(const char *key, const uint8_t *data, uint8_t len)
		{
			writeKey(key);
			printStr(F("\""));
			for (uint8_t i = 0; i < len; i++)
			{
				printHex(data[i]);
			}
			printStr(F("\""));
			return *this;
		}

		/**
		 * @brief Write a raw string fragment directly to the output stream
		 * @param fragment Raw JSON text to inject
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &raw(const char *fragment)
		{
			printStr(fragment);
			return *this;
		}

		/**
		 * @brief Write a raw flash-stored fragment directly to the output stream
		 * @param fragment Flash string pointer to raw JSON text
		 * @return Reference to this builder for chaining
		 */
		JsonObjectBuilder &raw(const __FlashStringHelper *fragment)
		{
			printStr(fragment);
			return *this;
		}

		/**
		 * @brief Merge fields from a callback into this object
		 * @param fields Callable that receives this builder and adds fields
		 * @return Reference to this builder for chaining
		 */
		template <typename Fn> JsonObjectBuilder &merge(Fn fields)
		{
			fields(*this);
			return *this;
		}

		/**
		 * @brief Merge fields from two callbacks into this object
		 * @param fieldsA First callable that adds fields
		 * @param fieldsB Second callable that adds fields
		 * @return Reference to this builder for chaining
		 */
		template <typename FnA, typename FnB> JsonObjectBuilder &merge(FnA fieldsA, FnB fieldsB)
		{
			fieldsA(*this);
			fieldsB(*this);
			return *this;
		}

		/**
		 * @brief Write a nested JSON object under the given key
		 * @param key JSON key name for the nested object
		 * @param fields Callable that populates the nested object
		 * @return Reference to this builder for chaining
		 */
		template <typename Fn> JsonObjectBuilder &object(const char *key, Fn fields)
		{
			writeKey(key);
			printStr(F("{"));
			JsonObjectBuilder nested;
			fields(nested);
			nested.close();
			return *this;
		}

		/**
		 * @brief Alias for object() — write a nested object under the given key
		 * @param key JSON key name for the nested object
		 * @param fields Callable that populates the nested object
		 * @return Reference to this builder for chaining
		 */
		template <typename Fn> JsonObjectBuilder &mergeObject(const char *key, Fn fields)
		{
			return object(key, fields);
		}

		/**
		 * @brief Write a nested object populated by two callbacks
		 * @param key JSON key name for the nested object
		 * @param fieldsA First callable that adds fields
		 * @param fieldsB Second callable that adds fields
		 * @return Reference to this builder for chaining
		 */
		template <typename FnA, typename FnB> JsonObjectBuilder &mergeObject(const char *key, FnA fieldsA, FnB fieldsB)
		{
			return object(key,
						  [&](JsonObjectBuilder &nested)
						  {
							  fieldsA(nested);
							  fieldsB(nested);
						  });
		}

		/**
		 * @brief Close this nested object by emitting the closing brace
		 */
		void close()
		{
			printStr(F("}"));
		}

	  private:
		bool first_; // Tracks whether a comma separator is needed before the next key

		/**
		 * @brief Emit a comma separator (if needed) and the quoted key with colon
		 * @param key JSON key name to write
		 */
		void writeKey(const char *key)
		{
			if (!first_)
			{
				printStr(F(","));
			}
			first_ = false;
			printStr(F("\""));
			printStr(key);
			printStr(F("\":"));
		}
	};

	/**
	 * @brief Write a string key-value pair
	 * @param key JSON key name
	 * @param value Null-terminated string value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &str(const char *key, const char *value)
	{
		writeKey(key);
		printStr(F("\""));
		printStr(value);
		printStr(F("\""));
		return *this;
	}

	/**
	 * @brief Write a string key-value pair with flash-stored value
	 * @param key JSON key name
	 * @param value Flash string pointer
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &str(const char *key, const __FlashStringHelper *value)
	{
		writeKey(key);
		printStr(F("\""));
		printStr(value);
		printStr(F("\""));
		return *this;
	}

	/**
	 * @brief Write a numeric key-value pair
	 * @param key JSON key name
	 * @param value Long integer value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &num(const char *key, long value)
	{
		writeKey(key);
		printNum(value);
		return *this;
	}

	/**
	 * @brief Write a numeric key-value pair (int overload)
	 * @param key JSON key name
	 * @param value Integer value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &num(const char *key, int value)
	{
		return num(key, (long)value);
	}

	/**
	 * @brief Write a numeric key-value pair (unsigned int overload)
	 * @param key JSON key name
	 * @param value Unsigned integer value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &num(const char *key, unsigned int value)
	{
		return num(key, (long)value);
	}

	/**
	 * @brief Write a numeric key-value pair (unsigned long overload)
	 * @param key JSON key name
	 * @param value Unsigned long value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &num(const char *key, unsigned long value)
	{
		writeKey(key);
		printNum((long)value);
		return *this;
	}

	/**
	 * @brief Write a boolean key-value pair as 1 or 0
	 * @param key JSON key name
	 * @param value Boolean value
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &boolean(const char *key, bool value)
	{
		writeKey(key);
		printNum(value ? 1 : 0);
		return *this;
	}

	/**
	 * @brief Write a hex-encoded byte array as a quoted string value
	 * @param key JSON key name
	 * @param data Pointer to byte array
	 * @param len Number of bytes to encode
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &hex(const char *key, const uint8_t *data, uint8_t len)
	{
		writeKey(key);
		printStr(F("\""));
		for (uint8_t i = 0; i < len; i++)
		{
			printHex(data[i]);
		}
		printStr(F("\""));
		return *this;
	}

	/**
	 * @brief Write a raw string fragment directly to the output stream
	 * @param fragment Raw JSON text to inject
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &raw(const char *fragment)
	{
		printStr(fragment);
		return *this;
	}

	/**
	 * @brief Write a raw flash-stored fragment directly to the output stream
	 * @param fragment Flash string pointer to raw JSON text
	 * @return Reference to this builder for chaining
	 */
	JsonLineBuilder &raw(const __FlashStringHelper *fragment)
	{
		printStr(fragment);
		return *this;
	}

	/**
	 * @brief Merge fields from a callback into this top-level object
	 * @param fields Callable that receives this builder and adds fields
	 * @return Reference to this builder for chaining
	 */
	template <typename Fn> JsonLineBuilder &merge(Fn fields)
	{
		fields(*this);
		return *this;
	}

	/**
	 * @brief Merge fields from two callbacks into this top-level object
	 * @param fieldsA First callable that adds fields
	 * @param fieldsB Second callable that adds fields
	 * @return Reference to this builder for chaining
	 */
	template <typename FnA, typename FnB> JsonLineBuilder &merge(FnA fieldsA, FnB fieldsB)
	{
		fieldsA(*this);
		fieldsB(*this);
		return *this;
	}

	/**
	 * @brief Write a nested JSON object under the given key
	 * @param key JSON key name for the nested object
	 * @param fields Callable that populates the nested object
	 * @return Reference to this builder for chaining
	 */
	template <typename Fn> JsonLineBuilder &object(const char *key, Fn fields)
	{
		writeKey(key);
		printStr(F("{"));
		JsonObjectBuilder nested;
		fields(nested);
		nested.close();
		return *this;
	}

	/**
	 * @brief Alias for object() — write a nested object under the given key
	 * @param key JSON key name for the nested object
	 * @param fields Callable that populates the nested object
	 * @return Reference to this builder for chaining
	 */
	template <typename Fn> JsonLineBuilder &mergeObject(const char *key, Fn fields)
	{
		return object(key, fields);
	}

	/**
	 * @brief Write a nested object populated by two callbacks
	 * @param key JSON key name for the nested object
	 * @param fieldsA First callable that adds fields
	 * @param fieldsB Second callable that adds fields
	 * @return Reference to this builder for chaining
	 */
	template <typename FnA, typename FnB> JsonLineBuilder &mergeObject(const char *key, FnA fieldsA, FnB fieldsB)
	{
		return object(key,
					  [&](JsonObjectBuilder &nested)
					  {
						  fieldsA(nested);
						  fieldsB(nested);
					  });
	}

	/**
	 * @brief Close the JSON line by emitting the closing brace and a newline
	 */
	void end()
	{
		printStr(F("}"));
		printLn();
	}

  private:
	bool first_; // Tracks whether a comma separator is needed before the next key

	/**
	 * @brief Emit a comma separator (if needed) and the quoted key with colon
	 * @param key JSON key name to write
	 */
	void writeKey(const char *key)
	{
		if (!first_)
		{
			printStr(F(","));
		}
		first_ = false;
		printStr(F("\""));
		printStr(key);
		printStr(F("\":"));
	}
};

/**
 * @brief Factory function that creates and returns a new JsonLineBuilder
 * @return A fresh JsonLineBuilder with the opening brace already emitted
 */
inline JsonLineBuilder jsonLine()
{
	return JsonLineBuilder();
}

/**
 * @brief Send an acknowledgement response for a successfully executed command
 * @param cmd The command name that was acknowledged
 */
void sendAck(const char *cmd)
{
	jsonLine().str("t", "ack").str("cmd", cmd).end();
}

/**
 * @brief Send an error message to the client
 * @param msg Error description (C string)
 */
void sendError(const char *msg)
{
	jsonLine().str("t", "error").str("msg", msg).end();
}

/**
 * @brief Send an error message to the client (flash string overload)
 * @param msg Error description stored in flash
 */
void sendError(const __FlashStringHelper *msg)
{
	jsonLine().str("t", "error").str("msg", msg).end();
}

/**
 * @brief Send a log message to the client
 * @param msg Log text (C string)
 */
void sendLog(const char *msg)
{
	jsonLine().str("t", "log").str("msg", msg).end();
}

/**
 * @brief Send a log message to the client (flash string overload)
 * @param msg Log text stored in flash
 */
void sendLog(const __FlashStringHelper *msg)
{
	jsonLine().str("t", "log").str("msg", msg).end();
}

/**
 * @brief Send a CAN frame event to the client if streaming is enabled
 * @param f CAN frame containing id, dlc, and data payload
 * @param dir Direction string ("rx" or "tx")
 * @param bus Bus index (0=chassis, 1=vehicle, 2=body)
 * @param ms Timestamp in milliseconds
 * @param s Transport state (checked for streamEnabled, increments streamCount)
 */
void sendFrame(const Frame &f, const char *dir, uint8_t bus, unsigned long ms, State &s)
{
	if (!s.streamEnabled)
		return;
	s.streamCount++;

	JsonLineBuilder line = jsonLine();
	line.str("t", "frame")
		.str("dir", dir)
		.num("bus", bus)
		.num("id", (unsigned long)f.id)
		.num("seq", s.streamCount)
		.num("ms", ms)
		.num("ext", 0)
		.num("dlc", f.dlc)
		.hex("d", f.data, f.dlc);
	line.end();
}

/**
 * @brief Extract the "cmd" value from a minimal JSON-RPC envelope {"cmd":"<method>"}
 * @param buf Input buffer containing the JSON string
 * @param out Output buffer to receive the extracted command name
 * @param maxLen Maximum length of the output buffer
 * @return true if a valid command string was extracted, false otherwise
 */
static bool parseRpcCmd(const char *buf, char *out, uint8_t maxLen)
{
	const char *p = strstr(buf, "\"cmd\"");
	if (!p) return false;
	p += 5; // Skip past "cmd" key
	while (*p == ' ' || *p == ':') p++; // Skip whitespace and colon
	if (*p != '"') return false;
	p++; // Skip opening quote of value
	uint8_t i = 0;
	while (*p && *p != '"' && i < (uint8_t)(maxLen - 1)) out[i++] = *p++;
	if (*p != '"' || i == 0) return false;
	out[i] = '\0';
	return true;
}

// Forward-declare executeCommand (defined in the platform serial header after this include)
void executeCommand(const char *cmd, State &s, unsigned long now);

/**
 * @brief Process a single incoming character from a serial transport
 *
 * Accumulates characters into the buffer until a newline is received, then
 * attempts to parse the buffer as a JSON-RPC command envelope.
 *
 * @param buf Character accumulation buffer
 * @param len Current number of bytes in the buffer (updated in place)
 * @param c The character just received
 * @param s Transport state passed to the command executor
 */
void handleChar(char *buf, uint8_t &len, char c, State &s)
{
	if (c == '\r')
		return; // Ignore carriage returns

	if (c == '\n')
	{
		if (len > 0 && len < SERIAL_CMD_BUFFER_SIZE)
		{
			buf[len] = '\0';
			char rpcCmd[SERIAL_CMD_BUFFER_SIZE];
			if (buf[0] == '{')
			{
				if (parseRpcCmd(buf, rpcCmd, sizeof(rpcCmd)))
				{
					executeCommand(rpcCmd, s, millis());
				}
				else
				{
					sendError(F("rpc: expected {\"cmd\":\"...\"}"));
				}
			}
			else
			{
				sendError(F("rpc: expected json object"));
			}
		}
		len = 0;
		return;
	}

	// Accept JSON structural characters plus the alphanumeric/punctuation set used in commands
	bool valid =
		(c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
		c == ':' || c == '-' || c == '_' ||
		c == '{' || c == '}' || c == '"' || c == ',' || c == ' ';
	if (!valid)
	{
		len = 0; // Invalid character resets the buffer
		return;
	}

	if (len < (SERIAL_CMD_BUFFER_SIZE - 1))
	{
		buf[len++] = c;
	}
	else
	{
		len = SERIAL_CMD_BUFFER_SIZE; // Mark buffer as overflowed
	}
}
