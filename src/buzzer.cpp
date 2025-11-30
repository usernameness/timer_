#include "../include/buzzer.hpp"

TaskHandle_t buzzer::buzzerTaskHandle = nullptr;

void buzzer::init() {
    pinMode(BUZZER_PIN, OUTPUT);

    xTaskCreate(buzzer::execute, "BuzzerTask", 2048, NULL, 3, &buzzerTaskHandle);
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
    digitalWrite(BUZZER_PIN, HIGH);
    delay(3);
    digitalWrite(BUZZER_PIN, LOW);
}

void buzzer::smallBeep() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(10);
    digitalWrite(BUZZER_PIN, LOW);
}

void buzzer::alarmBeeps() {
    for (int i = 0; i < 3; ++i) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(300);
        digitalWrite(BUZZER_PIN, LOW);
        delay(300);
    }
    delay(1000); // Pause between alarm beeps
}