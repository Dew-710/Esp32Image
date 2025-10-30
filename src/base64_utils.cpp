#include "base64_utils.h"
#include "mbedtls/base64.h"

uint8_t* decodeBase64(const String& input, size_t* outLen) {
  *outLen = (input.length() * 3) / 4;
  uint8_t* decoded = (uint8_t*)malloc(*outLen);
  if (!decoded) return nullptr;

  int res = mbedtls_base64_decode(decoded, *outLen, outLen,
                                  (const unsigned char*)input.c_str(),
                                  input.length());

  if (res != 0) {
    free(decoded);
    return nullptr;
  }

  return decoded;
}
