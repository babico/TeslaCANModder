CXX      ?= c++
BASE_CXXFLAGS := -std=c++14 -Wall -Wextra
CXXFLAGS := $(BASE_CXXFLAGS) -O2
SANITIZER_CXXFLAGS := $(BASE_CXXFLAGS) -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined

UNAME_S := $(shell uname -s)
SANITIZER_ENV :=

ifeq ($(UNAME_S),Linux)
SANITIZER_ENV := ASAN_OPTIONS=detect_leaks=1
endif

TEST_BINS := test_handlers test_handlers_sanitize

test_handlers: test_handlers.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

test_handlers_sanitize: test_handlers.cpp
	$(CXX) $(SANITIZER_CXXFLAGS) -o $@ $<

test: test_handlers
	./test_handlers

sanitize: test_handlers_sanitize
	$(SANITIZER_ENV) ./test_handlers_sanitize

clean:
	rm -f $(TEST_BINS)
	rm -rf *.dSYM

.PHONY: test sanitize clean
