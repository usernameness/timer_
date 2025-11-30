#include "../include/clock.hpp"

RTC_DS3231 RTC::rtc = RTC_DS3231();
DateTime RTC::now_ = DateTime();

void RTC::init() {

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

    now_ = rtc.now();
}

void RTC::execute(void *pvParameters) {

    now_ = rtc.now();
    delay(60000);
}

void RTC::handleSQWInterrupt() {
    now_ = now_ + TimeSpan(0, 0, 0, 1);
}

auto RTC::now() -> const DateTime {
    return now_;
}

void RTC::reset() {
    // Reset clock if needed
}