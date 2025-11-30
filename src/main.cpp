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

void setup() {

  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Halt if deiplay not found
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Hello, World!"); 
  display.display();

  clock_.init();
  buzzer_.init();
  encoder_.init(buzzer_.buzzerTaskHandle);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); 
  delay(500);                   
  digitalWrite(LED_PIN, LOW);    
  delay(500);
  
  DateTime now = clock_.now();

  // Clear screen before drawing
  display.clearDisplay();

  // Print time
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(now.hour());
  display.print(":");
  if (now.minute() < 10) display.print("0"); // leading zero
  display.print(now.minute());
  display.print(":");
  if (now.second() < 10) display.print("0");
  display.print(now.second());

  // Print date
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print(now.day());
  display.print("/");
  display.print(now.month());
  display.print("/");
  display.print(now.year());


  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Encoder Pos: ");
  display.print(encoder_.get_position());

  display.display();
}