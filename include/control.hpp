#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <RTClib.h>
#include "../include/encoder.hpp"
#include "../include/clock.hpp"



class control {
public:

    void init(encoder &encoderRef, RTC &clockRef);

    auto get_timers() -> std::array<DateTime, 4>;

    static TaskHandle_t controlTaskHandle;

private:
    void reset();
    static void execute(void *pvParameters);


    static std::array<DateTime, 4> timers_;

    static SemaphoreHandle_t mutex_;

    static encoder* encoderRef_;
    static RTC* clockRef_;

};