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

void setup() {

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  display_.init(encoder_, clock_);
  clock_.init();
  buzzer_.init();
  encoder_.init(buzzer_.buzzerTaskHandle, display_.displayTaskHandle);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); 
  delay(500);                   
  digitalWrite(LED_PIN, LOW);    
  delay(500);
}