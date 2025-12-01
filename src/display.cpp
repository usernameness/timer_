#include "../include/display.hpp"

Adafruit_SSD1306 display::display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
encoder* display::encoderRef_ = nullptr;
RTC* display::clockRef_ = nullptr;
TaskHandle_t display::displayTaskHandle = nullptr;

void display::init(encoder &encoderRef, RTC &clockRef) {
    encoderRef_ = &encoderRef;
    clockRef_ = &clockRef;

    if(!display_.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
                
        //Notify if display is not found - TODO: Handle error appropriately

        for(;;); // Halt if deiplay not found
    }

    display_.clearDisplay();
    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    display_.setCursor(0,0);
    display_.println("Initializing..."); 
    display_.display();

    xTaskCreate(display::execute, "Display Task", 2048, NULL, 1, NULL);
}

void display::execute(void *pvParameters) {
    for (;;) {

        DateTime now = clockRef_->now();

        // Clear screen before drawing
        display_.clearDisplay();

        /*
        // Print time
        display_.setCursor(0, 0);
        display_.setTextSize(2);
        display_.print(now.hour());
        display_.print(":");
        if (now.minute() < 10) display_.print("0");
        display_.print(now.minute());
        display_.print(":");
        if (now.second() < 10) display_.print("0");
        display_.print(now.second());

        // Print date
        display_.setTextSize(1);
        display_.setCursor(0, 30);
        display_.print(now.day());
        display_.print("/");
        display_.print(now.month());
        display_.print("/");
        display_.print(now.year());


        display_.setTextSize(1);
        display_.setCursor(0, 50);
        display_.print("Encoder Pos: ");
        //display_.print(encoderRef_->get_position());

        */

        static uint32_t x = 0;
        x++;
        
        display_.setTextSize(1);
        display_.setCursor(0, 0);
        display_.print("Ticks: ");
        display_.print(x);

        display_.display();



        vTaskDelay(pdMS_TO_TICKS(1000)); // Update every second
    }
}

void display::reset() {
    // Reset display state if needed
}
