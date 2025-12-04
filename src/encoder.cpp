// Include the concrete headers this translation unit needs
#include "../include/buzzer.hpp"
#include "../include/display.hpp"
#include "../include/encoder.hpp"
#include "../include/buzzer.hpp"

volatile int encoder::encoderPos = 0;
volatile int encoder::lastCLKState = 0;
volatile bool encoder::switchPressed = false;
TaskHandle_t encoder::buzzerTaskHandle = nullptr;
TaskHandle_t encoder::displayTaskHandle = nullptr;

// Mutex for task-level protection
SemaphoreHandle_t encoder::mutex_ = nullptr;
// Spinlock for ISR protection
portMUX_TYPE encoder::mux_ = portMUX_INITIALIZER_UNLOCKED;

//------------------------------------------------------------------------------------------------
void encoder::init(TaskHandle_t buzzerHandle, TaskHandle_t displayHandle) {

    buzzerTaskHandle = buzzerHandle;
    displayTaskHandle = displayHandle;

    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);

    lastCLKState = digitalRead(ENC_CLK);
    encoderPos = 0;
    switchPressed = false;

    mutex_ = xSemaphoreCreateMutex();

    attachInterrupt(digitalPinToInterrupt(ENC_CLK), handleEncoder, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(ENC_DT), handleEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_SW), handleSwitch, FALLING);
}

//------------------------------------------------------------------------------------------------
void encoder::execute() {
    // This function can be used to process encoder events if needed
}

//------------------------------------------------------------------------------------------------
void encoder::reset() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    encoderPos = 0;
    switchPressed = false;
    xSemaphoreGive(mutex_);
}

//------------------------------------------------------------------------------------------------
auto encoder::get_position() -> const int {
    int pos;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    pos = encoderPos;
    xSemaphoreGive(mutex_);
    return pos;
}

//------------------------------------------------------------------------------------------------
auto encoder::get_cursor_position() -> const int {
    int pos;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    pos = encoderPos;
    xSemaphoreGive(mutex_);
    return pos % 4;
}

//------------------------------------------------------------------------------------------------
auto encoder::is_switch_pressed() -> const bool {
    bool pressed;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    pressed = switchPressed;
    switchPressed = false; // clear after read if desired
    xSemaphoreGive(mutex_);
    return pressed;
}

//------------------------------------------------------------------------------------------------
void IRAM_ATTR encoder::handleEncoder() {


    static uint32_t lastInterruptTick = 0;
    uint32_t nowTick = xTaskGetTickCountFromISR();
    
    // Debounce: ignore if less than 2 ms since last interrupt
    if ((nowTick - lastInterruptTick) < pdMS_TO_TICKS(1)) {
        return;
    }
    lastInterruptTick = nowTick;

    // Direct GPIO register reads for ESP32-C3
    int currentCLKState = (GPIO.in.val >> ENC_CLK) & 0x1;
    int currentDTState  = (GPIO.in.val >> ENC_DT) & 0x1;

    if (currentCLKState != lastCLKState && currentCLKState == HIGH) {
        portENTER_CRITICAL_ISR(&mux_);
        if (currentDTState != currentCLKState) {
            encoderPos++;
        } else {
            encoderPos--;
        }
        portEXIT_CRITICAL_ISR(&mux_);


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

        // Notify Display: encoder moved
        if (displayTaskHandle != nullptr) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyFromISR(displayTaskHandle,
                               0,
                               eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }


    }

    lastCLKState = currentCLKState;

}

//------------------------------------------------------------------------------------------------
void IRAM_ATTR encoder::handleSwitch() {
    static uint32_t lastPressTick = 0;
    uint32_t nowTick = xTaskGetTickCountFromISR();

    if ((nowTick - lastPressTick) < pdMS_TO_TICKS(150)) {
        return;
    }
    lastPressTick = nowTick;

    portENTER_CRITICAL_ISR(&mux_);
    switchPressed = true;
    portEXIT_CRITICAL_ISR(&mux_);

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