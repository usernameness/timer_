#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <wire.h>
#include <RTClib.h>

#define SDA_PIN 6
#define SCL_PIN 5
#define SQW_PIN 7

class RTC {

public:
    void init();
    auto now() -> const DateTime;

private:

    static void execute(void *pvParameters);
    static void handleSQWInterrupt();
    void reset();

    static RTC_DS3231 rtc;
    static DateTime now_;
    static SemaphoreHandle_t mutex_;
    static portMUX_TYPE mux_;
};