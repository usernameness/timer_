#include <Arduino.h>
#include <wire.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>

#include "../include/buzzer.hpp"
#include "../include/encoder.hpp"
#include "../include/display.hpp"
#include "../include/clock.hpp"
#include "../include/control.hpp"


#define LED_PIN 8


encoder encoder_;
RTC clock_;
buzzer buzzer_;
display display_;
control control_;

void setup() {

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  clock_.init();
  buzzer_.init();
  encoder_.init(buzzer_.buzzerTaskHandle, display_.displayTaskHandle);
  
  control_.init(encoder_, clock_); 
  display_.init(encoder_, control_);

}

void loop() {
  GPIO.out_w1ts.val = (1 << 8);  // Set LED_PIN high for debugging
  delay(500);                   
  GPIO.out_w1tc.val = (1 << 8);  // Clear LED_PIN high after encoder move
  delay(500);
}