#include "../include/clock.hpp"

RTC_DS3231 RTC::rtc = RTC_DS3231();
DateTime RTC::now_ = DateTime();
SemaphoreHandle_t RTC::mutex_ = nullptr;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void RTC::init() {

    mutex_ = xSemaphoreCreateMutex();

    // Interrupt Service Routine for SQW pin is used to update time every second
    // as it is more accurate than relying on task delays and reading the RTC periodically
    // and lowers I2C bus usage
    pinMode(SQW_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SQW_PIN), handleSQWInterrupt, RISING);

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

    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);

    // Initial time read
    xSemaphoreTake(mutex_, portMAX_DELAY);
    now_ = rtc.now();
    xSemaphoreGive(mutex_);
}

void RTC::execute(void *pvParameters) {
    for(;;){
        DateTime t = rtc.now();
        xSemaphoreTake(mutex_, portMAX_DELAY);
        now_ = t;
        xSemaphoreGive(mutex_);
        delay(60000);
    }
}

void RTC::handleSQWInterrupt() {
    portENTER_CRITICAL_ISR(&RTC::mux_);
    now_ = now_ + TimeSpan(0, 0, 0, 1);
    portEXIT_CRITICAL_ISR(&RTC::mux_);
}

auto RTC::now() -> const DateTime {
    DateTime copy;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    copy = now_;
    xSemaphoreGive(mutex_);
    return copy;
}

void RTC::reset() {
    // Reset clock if needed
}