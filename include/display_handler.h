#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <TJpg_Decoder.h>

// Extern declaration cho tft (define ở main.cpp)
extern Adafruit_ST7735 tft;

// Extern declaration cho callback function
extern bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

// Function prototypes
void initDisplay();
void decodeAndDisplayJPEG(uint8_t* jpegBuffer, int jpegBufferLen);

#endif