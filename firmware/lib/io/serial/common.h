#pragma once
// Shared serial JSON message helpers.
// Include AFTER printStr/printNum/printHex/printLn are defined
// in the platform serial header.

#ifndef SERIAL_CMD_BUFFER_SIZE
#define SERIAL_CMD_BUFFER_SIZE 32
#endif

class JsonLineBuilder {
 public:
  JsonLineBuilder() : first_(true) { printStr(F("{")); }

  class JsonObjectBuilder {
   public:
    JsonObjectBuilder() : first_(true) {}

    JsonObjectBuilder& str(const char* key, const char* value) {
      writeKey(key);
      printStr(F("\""));
      printStr(value);
      printStr(F("\""));
      return *this;
    }

    JsonObjectBuilder& str(const char* key, const __FlashStringHelper* value) {
      writeKey(key);
      printStr(F("\""));
      printStr(value);
      printStr(F("\""));
      return *this;
    }

    JsonObjectBuilder& num(const char* key, long value) {
      writeKey(key);
      printNum(value);
      return *this;
    }

    JsonObjectBuilder& num(const char* key, int value) { return num(key, (long)value); }
    JsonObjectBuilder& num(const char* key, unsigned int value) { return num(key, (long)value); }
    JsonObjectBuilder& num(const char* key, unsigned long value) {
      writeKey(key);
      printNum((long)value);
      return *this;
    }

    JsonObjectBuilder& boolean(const char* key, bool value) {
      writeKey(key);
      printNum(value ? 1 : 0);
      return *this;
    }

    JsonObjectBuilder& hex(const char* key, const uint8_t* data, uint8_t len) {
      writeKey(key);
      printStr(F("\""));
      for (uint8_t i = 0; i < len; i++) {
        printHex(data[i]);
      }
      printStr(F("\""));
      return *this;
    }

    JsonObjectBuilder& raw(const char* fragment) {
      printStr(fragment);
      return *this;
    }

    JsonObjectBuilder& raw(const __FlashStringHelper* fragment) {
      printStr(fragment);
      return *this;
    }

    template <typename Fn>
    JsonObjectBuilder& merge(Fn fields) {
      fields(*this);
      return *this;
    }

    template <typename FnA, typename FnB>
    JsonObjectBuilder& merge(FnA fieldsA, FnB fieldsB) {
      fieldsA(*this);
      fieldsB(*this);
      return *this;
    }

    template <typename Fn>
    JsonObjectBuilder& object(const char* key, Fn fields) {
      writeKey(key);
      printStr(F("{"));
      JsonObjectBuilder nested;
      fields(nested);
      nested.close();
      return *this;
    }

    template <typename Fn>
    JsonObjectBuilder& mergeObject(const char* key, Fn fields) {
      return object(key, fields);
    }

    template <typename FnA, typename FnB>
    JsonObjectBuilder& mergeObject(const char* key, FnA fieldsA, FnB fieldsB) {
      return object(key, [&](JsonObjectBuilder& nested) {
        fieldsA(nested);
        fieldsB(nested);
      });
    }

    void close() { printStr(F("}")); }

   private:
    bool first_;

    void writeKey(const char* key) {
      if (!first_) {
        printStr(F(","));
      }
      first_ = false;
      printStr(F("\""));
      printStr(key);
      printStr(F("\":"));
    }
  };

  JsonLineBuilder& str(const char* key, const char* value) {
    writeKey(key);
    printStr(F("\""));
    printStr(value);
    printStr(F("\""));
    return *this;
  }

  JsonLineBuilder& str(const char* key, const __FlashStringHelper* value) {
    writeKey(key);
    printStr(F("\""));
    printStr(value);
    printStr(F("\""));
    return *this;
  }

  JsonLineBuilder& num(const char* key, long value) {
    writeKey(key);
    printNum(value);
    return *this;
  }

  JsonLineBuilder& num(const char* key, int value) {
    return num(key, (long)value);
  }

  JsonLineBuilder& num(const char* key, unsigned int value) {
    return num(key, (long)value);
  }

  JsonLineBuilder& num(const char* key, unsigned long value) {
    writeKey(key);
    printNum((long)value);
    return *this;
  }

  JsonLineBuilder& boolean(const char* key, bool value) {
    writeKey(key);
    printNum(value ? 1 : 0);
    return *this;
  }

  JsonLineBuilder& hex(const char* key, const uint8_t* data, uint8_t len) {
    writeKey(key);
    printStr(F("\""));
    for (uint8_t i = 0; i < len; i++) {
      printHex(data[i]);
    }
    printStr(F("\""));
    return *this;
  }

  JsonLineBuilder& raw(const char* fragment) {
    printStr(fragment);
    return *this;
  }

  JsonLineBuilder& raw(const __FlashStringHelper* fragment) {
    printStr(fragment);
    return *this;
  }

  template <typename Fn>
  JsonLineBuilder& merge(Fn fields) {
    fields(*this);
    return *this;
  }

  template <typename FnA, typename FnB>
  JsonLineBuilder& merge(FnA fieldsA, FnB fieldsB) {
    fieldsA(*this);
    fieldsB(*this);
    return *this;
  }

  template <typename Fn>
  JsonLineBuilder& object(const char* key, Fn fields) {
    writeKey(key);
    printStr(F("{"));
    JsonObjectBuilder nested;
    fields(nested);
    nested.close();
    return *this;
  }

  template <typename Fn>
  JsonLineBuilder& mergeObject(const char* key, Fn fields) {
    return object(key, fields);
  }

  template <typename FnA, typename FnB>
  JsonLineBuilder& mergeObject(const char* key, FnA fieldsA, FnB fieldsB) {
    return object(key, [&](JsonObjectBuilder& nested) {
      fieldsA(nested);
      fieldsB(nested);
    });
  }

  void end() {
    printStr(F("}"));
    printLn();
  }

 private:
  bool first_;

  void writeKey(const char* key) {
    if (!first_) {
      printStr(F(","));
    }
    first_ = false;
    printStr(F("\""));
    printStr(key);
    printStr(F("\":"));
  }
};

inline JsonLineBuilder jsonLine() { return JsonLineBuilder(); }

// ── Simple JSON Messages ─────────────────────────────────────────────────────
void sendAck(const char* cmd) {
  jsonLine().str("t", "ack").str("cmd", cmd).end();
}

void sendError(const char* msg) {
  jsonLine().str("t", "error").str("msg", msg).end();
}

void sendError(const __FlashStringHelper* msg) {
  jsonLine().str("t", "error").str("msg", msg).end();
}

void sendLog(const char* msg) {
  jsonLine().str("t", "log").str("msg", msg).end();
}

void sendLog(const __FlashStringHelper* msg) {
  jsonLine().str("t", "log").str("msg", msg).end();
}

void sendFrame(const Frame& f, const char* dir, uint8_t bus, unsigned long ms, State& s) {
  if (!s.streamEnabled) return;
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

// ── Character Handler ────────────────────────────────────────────────────────
// Forward-declare executeCommand (defined in platform serial header after this include).
void executeCommand(const char* cmd, State& s, unsigned long now);

void handleChar(char* buf, uint8_t& len, char c, State& s) {
  if (c == '\r') return;

  if (c == '\n') {
    if (len > 0 && len < SERIAL_CMD_BUFFER_SIZE) {
      buf[len] = '\0';
      executeCommand(buf, s, millis());
    }
    len = 0;
    return;
  }

  bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == ':' || c == '-' || c == '_';
  if (!valid) { len = 0; return; }

  if (len < (SERIAL_CMD_BUFFER_SIZE - 1)) {
    buf[len++] = c;
  } else {
    len = SERIAL_CMD_BUFFER_SIZE;
  }
}
