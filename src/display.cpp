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
int cursorPos = encoderRef_->get_cursor_position();

// Format time string
char timeStr[9]; // "HH:MM:SS"
snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
         now.hour(), now.minute(), now.second());

// Clear screen
display_.clearDisplay();
display_.setTextSize(1);   // Small text so four lines fit
display_.setTextColor(SSD1306_WHITE);

// Each character in size=1 is ~6 pixels wide, 8 pixels tall
int charWidth = 6;
int charHeight = 8;
int textWidth = strlen(timeStr) * charWidth;
int textHeight = charHeight;
int xPos = (SCREEN_WIDTH - textWidth) / 2; // Center horizontally

// Line Y positions
int lineY[4] = {2, 18, 34, 50};

// Draw four centered lines
for (int i = 0; i < 4; i++) {
    int yPos = lineY[i];

    // If this line is the cursor position, draw rectangle
    if (cursorPos == (i)) {
        display_.drawRect(xPos - 2, yPos - 2, textWidth + 4, textHeight + 4, SSD1306_WHITE);
    }

    display_.setCursor(xPos, yPos);
    display_.print(timeStr);
}

display_.display();



        vTaskDelay(pdMS_TO_TICKS(50)); // Update every second
    }
}

void display::reset() {
    // Reset display state if needed
}
