#include "display_handler.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Arduino.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



void initDisplay() {
  Wire.begin(21, 22); // SDA, SCL — tùy board ESP32 của bạn
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C hoặc 0x3D
    Serial.println("❌ Không tìm thấy OLED!");
    for(;;); // Dừng lại để dễ debug
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("✅ OLED OK");
  display.display();
}

void showText(const String& text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void showDecodedImage(const uint8_t* imgData, size_t imgSize) {
  // Giả sử dữ liệu đã là bitmap 128x64, 1 bit/pixel
  display.clearDisplay();
  display.drawBitmap(0, 0, imgData, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.display();
}
