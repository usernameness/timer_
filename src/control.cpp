#include "../include/control.hpp"

std::array<DateTime, 4> control::timers_ = {
    DateTime(), DateTime(), DateTime(), DateTime()
};

SemaphoreHandle_t control::mutex_ = nullptr;

TaskHandle_t control::controlTaskHandle = nullptr;

encoder* control::encoderRef_ = nullptr;
RTC* control::clockRef_ = nullptr;


void control::init(encoder &encoderRef, RTC &clockRef) {

    encoderRef_ = &encoderRef;
    clockRef_ = &clockRef;

    mutex_ = xSemaphoreCreateMutex();

    if (mutex_ == nullptr) {
        // Handle mutex creation failure
        for(;;); // Halt if mutex not created
    }

    xTaskCreate(control::execute, "Control Task", 2048, NULL, 1, &controlTaskHandle);

    // Initialization code for control can be added here
}

void control::execute(void *pvParameters) {
    for (;;)
    {
        int cursorPos = encoderRef_->get_cursor_position();
        

        switch (cursorPos) {
            case 0:
                // Handle timer one logic
                xSemaphoreTake(mutex_, portMAX_DELAY);
                timers_[0] = DateTime(0,0,0,1,1,1);//clockRef_->now();   // Example: set/update timer one
                xSemaphoreGive(mutex_);
                break;

            case 1:
                // Handle timer two logic
                xSemaphoreTake(mutex_, portMAX_DELAY);
                timers_[1] = clockRef_->now();
                xSemaphoreGive(mutex_);
                break;

            case 2:
             // Handle timer three logic
                xSemaphoreTake(mutex_, portMAX_DELAY);
                timers_[2] = DateTime(0,0,0,1,1,3);//clockRef_->now();
                xSemaphoreGive(mutex_);
                break;

            case 3:
                // Handle timer four logic
                xSemaphoreTake(mutex_, portMAX_DELAY);
                timers_[3] = DateTime(0,0,0,1,1,4);//clockRef_->now();
                xSemaphoreGive(mutex_);
                break;

            default:
                // Safety fallback
                xSemaphoreTake(mutex_, portMAX_DELAY);
                timers_[3] = DateTime(0,0,0,1,1,4);//clockRef_->now();
                xSemaphoreGive(mutex_);
                break;
        }

        delay(100); // Control loop delay
    }
    
    
    
}

auto control::get_timers() -> std::array<DateTime, 4> {
    std::array<DateTime, 4> copy;

    xSemaphoreTake(mutex_, portMAX_DELAY);
    copy = timers_;   // make a snapshot while locked
    xSemaphoreGive(mutex_);

    return copy; // return by value (safe copy)
}

void control::reset() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    for (auto &t : timers_) {
        t = DateTime(); // reset each timer
    }
    xSemaphoreGive(mutex_);
}