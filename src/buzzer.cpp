#include "../include/buzzer.hpp"

TaskHandle_t buzzer::buzzerTaskHandle = nullptr;

void buzzer::init() {
    pinMode(BUZZER_PIN, OUTPUT);

    xTaskCreate(buzzer::execute, "BuzzerTask", 2048, NULL, 1, &buzzerTaskHandle);
}

void buzzer::execute(void *pvParameters) {
    uint32_t notificationValue;
    bool alarmActive = false;

    for (;;) {
        if (!alarmActive) {
            // Wait for a notification
            xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY);

            switch (notificationValue) {
                case ENCODER_MOVE:
                    shortBurst();
                    break;
                case BUTTON_PRESS:
                    smallBeep();
                    break;
                case ALARM_START:
                    alarmActive = true;
                    break;
            }
        } else {
            // Alarm mode: keep beeping until stopped
            alarmBeeps();

            // Non-blocking check for stop signal
            if (xTaskNotifyWait(0, 0, &notificationValue, 0) == pdTRUE) {
                if (notificationValue == ALARM_STOP) {
                    alarmActive = false;
                }
            }
        }

    }
}

void buzzer::shortBurst() {
    // Set pin HIGH
    GPIO.out_w1ts.val = (1 << BUZZER_PIN);
    vTaskDelay(pdMS_TO_TICKS(3));
    // Set pin LOW
    GPIO.out_w1tc.val = (1 << BUZZER_PIN);
}

void buzzer::smallBeep() {
    GPIO.out_w1ts.val = (1 << BUZZER_PIN);
    vTaskDelay(pdMS_TO_TICKS(10));
    GPIO.out_w1tc.val = (1 << BUZZER_PIN);
}

void buzzer::alarmBeeps() {
    for (int i = 0; i < 3; ++i) {
        GPIO.out_w1ts.val = (1 << BUZZER_PIN);
        vTaskDelay(pdMS_TO_TICKS(300));
        GPIO.out_w1tc.val = (1 << BUZZER_PIN);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // Pause between alarm beeps
}