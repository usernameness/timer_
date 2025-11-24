#include <Arduino.h>
#include <wire.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>


// --- Pin definitions ---
#define SDA_PIN 6
#define SCL_PIN 5
#define ENC_A   9
#define ENC_B   10
#define ENC_SW  4
#define LED_PIN 8

// OLED Display Defines
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RTC_DS3231 rtc;

void setup() {

  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);

  if(!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    for(;;); // Halt if deiplay not found
  }

  if(rtc.lostPower()) {
    Serial.println(F("RTC lost power, setting the time!"));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

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
}

void loop() {
  digitalWrite(LED_PIN, HIGH); 
  delay(500);                   
  digitalWrite(LED_PIN, LOW);    
  delay(500);
  
  DateTime now = rtc.now();

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

  display.display();
}