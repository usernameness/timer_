// Include concrete headers required by this .cpp
#include "../include/control.hpp"
#include "../include/encoder.hpp"
#include "../include/buzzer.hpp"
#include "../include/display.hpp"
#include "esp_sleep.h"

//------------------------------------------------------------------------------------------------
//Initialization of static members
std::array<DateTime, 4> control::timers_ = { DateTime(), DateTime(), DateTime(), DateTime()};
std::array<bool, 4> control::alarmsBlocked_ = { true, true, true, true };
SemaphoreHandle_t control::mutex_ = nullptr;
TaskHandle_t control::controlTaskHandle = nullptr;
encoder* control::encoderRef_ = nullptr;
RTC* control::clockRef_ = nullptr;
DateTime control::prevNow_ = DateTime();
DateTime control::inaticveTime_ = DateTime();

//------------------------------------------------------------------------------------------------
void control::init(encoder &encoderRef, RTC &clockRef) {

    encoderRef_ = &encoderRef;
    clockRef_ = &clockRef;

    mutex_ = xSemaphoreCreateMutex();

    if (mutex_ == nullptr) {
        // Handle mutex creation failure
        for(;;); // Halt if mutex not created
    }

    inaticveTime_ = clockRef_->now();

    xTaskCreate(control::execute, "Control Task", 2048, NULL, 1, &controlTaskHandle);

}

//------------------------------------------------------------------------------------------------
void control::execute(void *pvParameters) {
    int rotaryPos = 0;
    int cursorPos = 0;
    bool buttonPressed = false;
    int cursorPosAtSelction = 0;
    int encoderPosAtSelect = 0;
    bool selected = false;
    bool selectorBlocked = false;

    for (;;)
    {
        rotaryPos = encoderRef_->get_position();
        cursorPos = encoderRef_->get_cursor_position();
        buttonPressed = encoderRef_->is_switch_pressed();

        // If the button is pressed, toggle selection state
        // don't allow selection if selector is blocked (during alarm)
        // (allow unselecting to stop alarm)
        if (buttonPressed && !selected && !selectorBlocked) {
            encoderPosAtSelect = rotaryPos;
            cursorPosAtSelction = cursorPos;
            selected = true;
            xTaskNotify(display::displayTaskHandle, display::SELECTION, eSetValueWithOverwrite);
            GPIO.out_w1tc.val = (1 << 8);
        } else if (buttonPressed && selected) {
            selected = false;
            xTaskNotify(display::displayTaskHandle, display::UNSELECTION, eSetValueWithOverwrite);
            GPIO.out_w1ts.val = (1 << 8);

        }

        DateTime currentTime = clockRef_->now();

        if (selected) {
            // Adjust the selected timer based on encoder movement
            int delta = rotaryPos - encoderPosAtSelect;

            cursorPos = cursorPosAtSelction; // Keep cursor position fixed during selection

            xSemaphoreTake(mutex_, portMAX_DELAY);
            timers_[cursorPos] = timers_[cursorPos] + TimeSpan(0,0,0,delta);
            xSemaphoreGive(mutex_);

            encoderPosAtSelect = rotaryPos; // Update for next iteration
        }

       

        if (currentTime != prevNow_) {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            //If the current time has advanced, decrement timers that are not selected
            
            for (int i = 0; i < 4; ++i) {
                if (timers_[i] <= currentTime) {
                    timers_[i] = currentTime; // Prevent underflow
                } else {
                    timers_[i] = ((i == cursorPos) && selected) ? timers_[i] + TimeSpan(1) : timers_[i]; //- TimeSpan(0, 0, 0, 1);
                }
            }

            xSemaphoreGive(mutex_);
        }

        prevNow_ = currentTime;


        std::array<bool,4> alarmTriggered = {false, false, false, false};

        for (int i = 0; i < 4; ++i) {

            //unblock alarm if timer is set to future time else keep current state
            alarmsBlocked_[i] = (timers_[i] > currentTime) ? false : alarmsBlocked_[i];

            //check if alarm should trigger
            alarmTriggered[i] = (timers_[i] <= currentTime) && !alarmsBlocked_[i];
        }
        

        for (int i = 0; i < 4; ++i) {
            if (alarmTriggered[i]) {
            //block selector
            selectorBlocked = true;

            // Notify buzzer to start alarm
            if (buzzer::buzzerTaskHandle != nullptr) {
                xTaskNotify(buzzer::buzzerTaskHandle, buzzer::ALARM_START, eSetValueWithOverwrite);
            }
            }
        }

        for (int i = 0; i < 4; ++i) {
            if (buttonPressed && alarmTriggered[i]) {
                // Unblock selector
                selectorBlocked = false;

                alarmsBlocked_[i] = true;

                // Notify buzzer to stop alarm
                if (buzzer::buzzerTaskHandle != nullptr) {
                    xTaskNotify(buzzer::buzzerTaskHandle,buzzer::ALARM_STOP,eSetValueWithOverwrite);
                }
            }
        }


        //If no alarms are active for long time, go into low power mode


        bool timersInactive = false;
        for (int i = 0; i < 4; ++i) {

            //if any timer is active, break
            // dont set inactive time if all timers are inactive
            if (timers_[i] <= currentTime && alarmsBlocked_[i]) {
                timersInactive = true;
            } else {
                timersInactive = false;
                inaticveTime_ = currentTime;
                break;
            }
        }

        if (timersInactive) {
            //No alarms active
            //Check inactivity
            if (inaticveTime_.isValid()) {
                //Already tracking inactivity
                TimeSpan inactiveDuration = currentTime - inaticveTime_;
                if (inactiveDuration.totalseconds() >= 30) {
                    // Inactivity for 5 minutes, enter low power mode
                    display::turnDisplayOff();
                    // Bit mask for GPIO9
                    uint64_t mask = 1ULL << ENC_SW;

                    // Wake on falling edge (button press)
                    esp_deep_sleep_enable_gpio_wakeup(mask, ESP_GPIO_WAKEUP_GPIO_LOW);

                    esp_deep_sleep_start();
                }
            }
        } else {
            //Reset inactivity timer
            inaticveTime_ = currentTime;
        }

        //Turn debug LED off
        //Turn display off
        //put CPU to deep sleep, wake on encoder interrupt





        delay(100); // Control loop delay
    }
    
    
    
}

//------------------------------------------------------------------------------------------------
auto control::get_timers() -> std::array<TimeSpan, 4> {
    std::array<TimeSpan, 4> copy;

    xSemaphoreTake(mutex_, portMAX_DELAY);
    for (int i = 0; i < 4; ++i) {
        //Use TimeSpan difference to get remaining time
        // using previous time as it is safe against time
        // increasing betweeen executions cause display errors
        copy[i] = timers_[i] - prevNow_;
        //copy[i] = (copy[i].totalseconds() < 0) ? TimeSpan(0) : copy[i];
    }
    xSemaphoreGive(mutex_);

    return copy; // return by value (safe copy)
}

//------------------------------------------------------------------------------------------------
void control::reset() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    for (auto &t : timers_) {
        t = DateTime(); // reset each timer
    }
    xSemaphoreGive(mutex_);
}