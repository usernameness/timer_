#include "../include/clock.hpp"

RTC_DS3231 RTC::rtc = RTC_DS3231();
DateTime RTC::now_ = DateTime();
SemaphoreHandle_t RTC::mutex_ = nullptr;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//------------------------------------------------------------------------------------------------
void RTC::init() {

    mutex_ = xSemaphoreCreateMutex();

    // Create a task to periodically update the current time from RTC to avoid drift
    xTaskCreate(RTC::execute, "Clock Task", 2048, NULL, 1, NULL);

    if(!rtc.begin()) {
        
        //Notify if RTC is not found - TODO: Handle error appropriately

        for(;;); // Halt if RTC not found
    }

    if(rtc.lostPower()) {

        //Notify if RTC lost power - TODO: Handle error appropriately

        //RTC lost power, set time to compile time
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }


    // Initial time read
    xSemaphoreTake(mutex_, portMAX_DELAY);
    now_ = rtc.now();
    xSemaphoreGive(mutex_);
}

//------------------------------------------------------------------------------------------------
void RTC::execute(void *pvParameters) {
    for(;;){
        DateTime t = rtc.now();
        xSemaphoreTake(mutex_, portMAX_DELAY);
        now_ = t;
        xSemaphoreGive(mutex_);
        delay(50);
    }
}

//------------------------------------------------------------------------------------------------
auto RTC::now() -> const DateTime {
    DateTime copy;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    copy = now_;
    xSemaphoreGive(mutex_);
    return copy;
}

//------------------------------------------------------------------------------------------------
void RTC::reset() {
    // Reset clock if needed
}