#pragma once
#include <Arduino.h>



void initDisplay();
void showText(const String& text);
void showDecodedImage(const uint8_t* imgData, size_t imgSize);
