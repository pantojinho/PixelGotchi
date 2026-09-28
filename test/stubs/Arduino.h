#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
constexpr bool HIGH = true, LOW = false;
constexpr int INPUT_PULLUP = 2;
extern uint32_t testMs;
extern bool testButton;
inline uint32_t millis() { return testMs; }
inline bool digitalRead(int) { return testButton; }
inline void pinMode(int, int) {}
struct TestSerial {
    template <typename... T> void printf(const char *, T...) {}
    void println(const char *) {}
};
inline TestSerial Serial;
struct TestEsp { uint64_t getEfuseMac() { return 42; } };
inline TestEsp ESP;
