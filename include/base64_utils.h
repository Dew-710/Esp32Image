#pragma once
#include <Arduino.h>

uint8_t* decodeBase64(const String& input, size_t* outLen);
