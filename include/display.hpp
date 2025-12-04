#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <Adafruit_SSD1306.h>

// Forward declarations to avoid circular includes
class encoder;
class control;


// OLED Display Defines
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_ADDR   0x3C


class display {
public:

    enum Event : uint32_t {
        ENCODER_MOVE = 1,
        SELECTION = 2,
        UNSELECTION = 3
    };

    void init(encoder &encoderRef, control &controlRef);

    static void turnDisplayOff();
    static void turnDisplayOn();

    static TaskHandle_t displayTaskHandle;

private:
    static void execute(void *pvParameters);
    void reset();

    static encoder* encoderRef_;
    static control* controlRef_;

    static Adafruit_SSD1306 display_;

};