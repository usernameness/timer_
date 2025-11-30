#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>

// OLED Display Defines
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

class display {
public:

    void init();
    

private:

};