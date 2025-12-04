#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <RTClib.h>
#include "../include/clock.hpp"

// Forward declare encoder to avoid circular include
class encoder;


class control {
public:

    void init(encoder& encoderRef, RTC& clockRef);

    auto get_timers() -> std::array<DateTime, 4>;

    static TaskHandle_t controlTaskHandle;

private:
    void reset();
    static void execute(void *pvParameters);

    static DateTime prevNow_;

    static std::array<DateTime, 4> timers_;
    static std::array<bool, 4> alarmsBlocked_;



    static SemaphoreHandle_t mutex_;

    static encoder* encoderRef_;
    static RTC* clockRef_;
};