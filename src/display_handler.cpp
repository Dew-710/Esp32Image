#include "display_handler.h"

// Define tft_output ở đây (duy nhất một nơi)
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Dừng nếu vượt chiều cao màn hình
  if (y >= tft.height()) return 0;
  
  // Vẽ block RGB565
  tft.drawRGBBitmap(x, y, bitmap, w, h);
  
  // Tiếp tục decode
  return 1;
}

void initDisplay() {
  // Pin cho ST7735S
  #define TFT_CS     5
  #define TFT_RST    4
  #define TFT_DC     2
  #define TFT_MOSI   23
  #define TFT_SCLK   18

  // Init tft (dùng extern tft từ main)
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);  // Landscape 160x128
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
}

void decodeAndDisplayJPEG(uint8_t* jpegBuffer, int jpegBufferLen) {
  tft.fillScreen(ST77XX_BLACK);
  
  // Lấy kích thước JPEG trước khi vẽ
  uint16_t jpegWidth, jpegHeight;
  if (TJpgDec.getJpgSize(&jpegWidth, &jpegHeight, jpegBuffer, jpegBufferLen) != JDR_OK) {
    Serial.println("getJpgSize failed!");
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);
    tft.println("Size failed!");
    return;
  }
  
  Serial.printf("JPEG dimensions: %d x %d\n", jpegWidth, jpegHeight);
  
  TJpgDec.setJpgScale(1);  // Scale 1:1 (thay 2 nếu hình lớn)
  TJpgDec.setCallback(tft_output);
  
  // Vẽ JPEG và đo thời gian
  uint32_t start = millis();
  if (TJpgDec.drawJpg(0, 0, jpegBuffer, jpegBufferLen) == JDR_OK) {
    uint32_t drawTime = millis() - start;
    Serial.printf("Render time: %lu ms\n", drawTime);
  } else {
    Serial.println("JPEG draw failed!");
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);
    tft.println("Draw failed!");
  }
}