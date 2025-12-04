// Include concrete headers required by this .cpp
#include "../include/control.hpp"
#include "../include/encoder.hpp"
#include "../include/buzzer.hpp"
#include "../include/display.hpp"

//------------------------------------------------------------------------------------------------
//Initialization of static members
std::array<DateTime, 4> control::timers_ = { DateTime(), DateTime(), DateTime(), DateTime()};
std::array<bool, 4> control::alarmsBlocked_ = { true, true, true, true };
SemaphoreHandle_t control::mutex_ = nullptr;
TaskHandle_t control::controlTaskHandle = nullptr;
encoder* control::encoderRef_ = nullptr;
RTC* control::clockRef_ = nullptr;
DateTime control::prevNow_ = DateTime();

//------------------------------------------------------------------------------------------------
void control::init(encoder &encoderRef, RTC &clockRef) {

    encoderRef_ = &encoderRef;
    clockRef_ = &clockRef;

    mutex_ = xSemaphoreCreateMutex();

    if (mutex_ == nullptr) {
        // Handle mutex creation failure
        for(;;); // Halt if mutex not created
    }

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
            alarmsBlocked_[cursorPos] = false;
            xTaskNotify(display::displayTaskHandle, display::SELECTION, eSetValueWithOverwrite);
            GPIO.out_w1tc.val = (1 << 8);

        } else if (buttonPressed && selected) {
            selected = false;
            xTaskNotify(display::displayTaskHandle, display::UNSELECTION, eSetValueWithOverwrite);
            GPIO.out_w1ts.val = (1 << 8);

        }

        if (selected) {
            // Adjust the selected timer based on encoder movement
            int delta = rotaryPos - encoderPosAtSelect;

            cursorPos = cursorPosAtSelction; // Keep cursor position fixed during selection

            xSemaphoreTake(mutex_, portMAX_DELAY);
            timers_[cursorPos] = timers_[cursorPos] + TimeSpan(0,0,0,delta);
            xSemaphoreGive(mutex_);

            encoderPosAtSelect = rotaryPos; // Update for next iteration
        }

        //timer decrements
        DateTime currentTime = clockRef_->now();

        if (currentTime != prevNow_) {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            //If the current time has advanced, decrement timers that are not selected

            //TO DO ADD IF STATEMENT TO PREVENT UNDERFLOW

            timers_[0] = ((cursorPos == 0)&&(selected)) ? timers_[0]  : timers_[0] - TimeSpan(0, 0, 0, 1);
            timers_[1] = ((cursorPos == 1)&&(selected)) ? timers_[1]  : timers_[1] - TimeSpan(0, 0, 0, 1);
            timers_[2] = ((cursorPos == 2)&&(selected)) ? timers_[2]  : timers_[2] - TimeSpan(0, 0, 0, 1);
            timers_[3] = ((cursorPos == 3)&&(selected)) ? timers_[3]  : timers_[3] - TimeSpan(0, 0, 0, 1);
            xSemaphoreGive(mutex_);
        }

        prevNow_ = currentTime;

        //if any timer reached zero and alarm not blocked, trigger alarm
        bool alarmTriggered = false;
        alarmTriggered = (timers_[0] <= currentTime) && !alarmsBlocked_[0];
        alarmTriggered = alarmTriggered || ((timers_[1] <= currentTime) && !alarmsBlocked_[1]);
        alarmTriggered = alarmTriggered || ((timers_[2] <= currentTime) && !alarmsBlocked_[2]);
        alarmTriggered = alarmTriggered || ((timers_[3] <= currentTime) && !alarmsBlocked_[3]);
        

        if (alarmTriggered) {
            //block selector
            selectorBlocked = true;

            // Notify buzzer to start alarm
            if (buzzer::buzzerTaskHandle != nullptr) {
                xTaskNotify(buzzer::buzzerTaskHandle,buzzer::ALARM_START,eSetValueWithOverwrite);
            }
        }

        if (buttonPressed && alarmTriggered) {
            // Unblock selector
            selectorBlocked = false;

            alarmsBlocked_[0] = true;
            alarmsBlocked_[1] = true;
            alarmsBlocked_[2] = true;
            alarmsBlocked_[3] = true;

            // Notify buzzer to stop alarm
            if (buzzer::buzzerTaskHandle != nullptr) {
                xTaskNotify(buzzer::buzzerTaskHandle,buzzer::ALARM_STOP,eSetValueWithOverwrite);
            }
        }


        
        //alarms
        delay(100); // Control loop delay
    }
    
    
    
}

//------------------------------------------------------------------------------------------------
auto control::get_timers() -> std::array<DateTime, 4> {
    std::array<DateTime, 4> copy;

    xSemaphoreTake(mutex_, portMAX_DELAY);
    copy = timers_;   // make a snapshot while locked
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