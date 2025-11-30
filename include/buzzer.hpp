#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>

#define BUZZER_PIN 3

class buzzer{
public:

    void init();

    // Notification codes
    enum Event : uint32_t {
        ENCODER_MOVE = 1,
        BUTTON_PRESS = 2,
        ALARM_START  = 3,
        ALARM_STOP   = 4
    };

    static TaskHandle_t buzzerTaskHandle;

private:
    static void execute(void *pvParameters);
    static void shortBurst();
    static void smallBeep();
    static void alarmBeeps();
};

