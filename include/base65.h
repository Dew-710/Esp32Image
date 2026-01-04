#ifndef BASE64_H
#define BASE64_H

#include <Arduino.h>

int base64_dec_len(const char *input, int inputLen);
int base64_decode(unsigned char *output, size_t outputSize, size_t *outLen,
                  const unsigned char *input, size_t inputLen);


#endif
