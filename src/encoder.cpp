#include "../include/encoder.hpp"
#include "../include/buzzer.hpp"

volatile int encoder::encoderPos = 0;
volatile int encoder::lastCLKState = 0;
volatile bool encoder::switchPressed = false;
TaskHandle_t encoder::buzzerTaskHandle = nullptr;
TaskHandle_t encoder::displayTaskHandle = nullptr;

void encoder::init(TaskHandle_t buzzerHandle, TaskHandle_t displayHandle) {

    buzzerTaskHandle = buzzerHandle;
    displayTaskHandle = displayHandle;

    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);

    lastCLKState = digitalRead(ENC_CLK);
    encoderPos = 0;
    switchPressed = false;

    attachInterrupt(digitalPinToInterrupt(ENC_CLK), handleEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_DT), handleEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_SW), handleSwitch, FALLING);
}

void encoder::execute() {
    // This function can be used to process encoder events if needed
}

void encoder::reset() {
    encoderPos = 0;
    switchPressed = false;
}

auto encoder::get_position() -> const int {
    return encoderPos;
}

auto encoder::get_cursor_position() -> const int {
    return encoderPos % 4;
}

auto encoder::is_switch_pressed() -> const bool {
    return switchPressed;
}

void IRAM_ATTR encoder::handleEncoder() {
    int currentCLKState = digitalRead(ENC_CLK);
    int currentDTState  = digitalRead(ENC_DT);

    if (currentCLKState != lastCLKState && currentCLKState == HIGH) {
        if (currentDTState != currentCLKState) {
            encoderPos++;
        } else {
            encoderPos--;
        }

        // Notify buzzer: encoder moved
        if (buzzerTaskHandle != nullptr) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyFromISR(buzzerTaskHandle,
                               buzzer::ENCODER_MOVE,
                               eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }
    }

    lastCLKState = currentCLKState;
}

void IRAM_ATTR encoder::handleSwitch() {
    static uint32_t lastPressTick = 0;
    uint32_t nowTick = xTaskGetTickCountFromISR();

    if ((nowTick - lastPressTick) < pdMS_TO_TICKS(150)) {
        return;
    }
    lastPressTick = nowTick;

    switchPressed = true;

    // Notify buzzer: button pressed
    if (buzzerTaskHandle != nullptr) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(buzzerTaskHandle,
                           buzzer::BUTTON_PRESS,
                           eSetValueWithOverwrite,
                           &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}