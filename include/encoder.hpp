#pragma once

#include <Arduino.h>
#include <cstdint>
#include <functional>

#define ENC_CLK   9
#define ENC_DT   10
#define ENC_SW  4

class encoder {
public:

    void init(TaskHandle_t buzzerHandle);


    [[nodiscard]] auto get_position() -> const int;
    [[nodiscard]] auto get_cursor_position() -> const int;
    [[nodiscard]] auto is_switch_pressed() -> const bool;
    
private:

    void execute();
    void reset();

    static void IRAM_ATTR handleEncoder();
    static void IRAM_ATTR handleSwitch();


    static volatile int lastCLKState;
    static volatile int encoderPos;
    static volatile bool switchPressed;
    static TaskHandle_t buzzerTaskHandle;
};
