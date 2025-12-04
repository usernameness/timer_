// Include concrete headers required by this translation unit
#include "../include/display.hpp"
#include "../include/encoder.hpp"
#include "../include/control.hpp"

Adafruit_SSD1306 display::display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
encoder* display::encoderRef_ = nullptr;
control* display::controlRef_ = nullptr;
TaskHandle_t display::displayTaskHandle = nullptr;

//------------------------------------------------------------------------------------------------
void display::init(encoder &encoderRef, control &controlRef) {
    encoderRef_ = &encoderRef;
    controlRef_ = &controlRef;

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

    xTaskCreate(display::execute, "Display Task", 2048, NULL, 1, &displayTaskHandle);
}

//------------------------------------------------------------------------------------------------
void display::execute(void *pvParameters) {
    uint32_t notificationValue;
    int cursorPosAtSelction = 0;
    bool selectionActive = false;


    for (;;) {

        xTaskNotifyWait(0, UINT32_MAX, &notificationValue, pdMS_TO_TICKS(50));

        std::array<DateTime,4> now = controlRef_->get_timers(); // For now, just show the first timer
        int cursorPos = encoderRef_->get_cursor_position();

        if(notificationValue == ENCODER_MOVE) {
            // Handle encoder move if needed
        } else if (notificationValue == SELECTION) {
            cursorPosAtSelction = cursorPos;
            selectionActive = true;

        } else if (notificationValue == UNSELECTION) {
            // Handle unselection if needed
            selectionActive = false;
            cursorPosAtSelction = 0;
        }

        if (selectionActive) {
            cursorPos = cursorPosAtSelction;
        }

        // Clear screen
        display_.clearDisplay();
        display_.setTextSize(1);   // Small text so four lines fit
        display_.setTextColor(SSD1306_WHITE);

       

        // Line Y positions
        int lineY[4] = {2, 18, 34, 50};

        //Initialize variables outside of for loop
        char timeStr[9]; // "HH:MM:SS"
        int charWidth = 6;
        int charHeight = 8;
        int textHeight = charHeight;
        int textWidth;
        int xPos, yPos;


        // Draw four centered lines
        for (int i = 0; i < 4; i++) {

            // Format time string
             snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
                 now[i].hour(), now[i].minute(), now[i].second());

            textWidth = strlen(timeStr) * charWidth;
            xPos = (SCREEN_WIDTH - textWidth) / 2; // Center horizontally    
            yPos = lineY[i];

            // If this line is the cursor position, draw rectangle
            if (cursorPos == (i)) {
                display_.drawRect(xPos - 2, yPos - 2, textWidth + 4, textHeight + 4, SSD1306_WHITE);
            }

            display_.setCursor(xPos, yPos);
            display_.print(timeStr);
        }

        display_.display();

    }
}

//------------------------------------------------------------------------------------------------
void display::reset() {
    // Reset display state if needed
}
