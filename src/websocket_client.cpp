#include "websocket_client.h"
#include "config.h"
#include <FS.h>
#include <SPIFFS.h>
#include "mbedtls/base64.h"
#include "display_handler.h"

WebSocketsClient webSocket;
File imgFile;
const size_t CHUNK_MAX = 8192;

// -------------------------------------
// Decode Base64 file safely
// -------------------------------------
void decodeTask(void *pvParameters) {
  Serial.println("🔧 decodeTask started");
  Serial.printf("Free heap before decode: %u bytes\n", ESP.getFreeHeap());

  File in = SPIFFS.open("/image_base64.txt", FILE_READ);
  if (!in) {
    Serial.println("❌ No base64 file found");
    vTaskDelete(NULL);
    return;
  }

  File out = SPIFFS.open("/image.jpg", FILE_WRITE);
  if (!out) {
    Serial.println("❌ Cannot open /image.jpg for write");
    in.close();
    vTaskDelete(NULL);
    return;
  }

  const size_t BLOCK_SIZE = 4096;
  char *inBuf = (char*) malloc(BLOCK_SIZE + 5);
  unsigned char *outBuf = (unsigned char*) malloc((BLOCK_SIZE / 4) * 3 + 8);
  if (!inBuf || !outBuf) {
    Serial.println("❌ Not enough heap for decode buffers");
    if (inBuf) free(inBuf);
    if (outBuf) free(outBuf);
    in.close();
    out.close();
    vTaskDelete(NULL);
    return;
  }

  String carry = "";
  size_t bytesRead;

  while ((bytesRead = in.readBytes(inBuf, BLOCK_SIZE)) > 0) {
    inBuf[bytesRead] = '\0';
    String chunk = carry + String(inBuf);
    carry = "";

    int remainder = chunk.length() % 4;
    if (remainder != 0) {
      carry = chunk.substring(chunk.length() - remainder);
      chunk = chunk.substring(0, chunk.length() - remainder);
    }

    size_t decodedLen = 0;
    int ret = mbedtls_base64_decode(outBuf, (BLOCK_SIZE / 4) * 3 + 8,
                                    &decodedLen,
                                    (unsigned char*)chunk.c_str(),
                                    chunk.length());
    if (ret == 0 && decodedLen > 0) {
      out.write(outBuf, decodedLen);
    } else {
      Serial.printf("⚠️ Decode error: ret=%d len=%d\n", ret, chunk.length());
    }
  }

  if (carry.length() > 0) {
    size_t decodedLen = 0;
    int ret = mbedtls_base64_decode(outBuf, (BLOCK_SIZE / 4) * 3 + 8,
                                    &decodedLen,
                                    (unsigned char*)carry.c_str(),
                                    carry.length());
    if (ret == 0 && decodedLen > 0) {
      out.write(outBuf, decodedLen);
    }
  }

  free(inBuf);
  free(outBuf);
  in.close();
  out.close();

  Serial.println("✅ Image decoded and saved → /image.jpg");
  Serial.printf("Free heap after decode: %u bytes\n", ESP.getFreeHeap());

  SPIFFS.remove("/image_base64.txt");

  // 🖼️ Hiển thị ảnh lên OLED
  File img = SPIFFS.open("/image.jpg", FILE_READ);
  if (img) {
    size_t imgSize = img.size();
    uint8_t *imgData = (uint8_t*) malloc(imgSize);
    if (imgData) {
      img.read(imgData, imgSize);
      showDecodedImage(imgData, imgSize);
      free(imgData);
      Serial.println("🖼️ Image displayed on OLED");
    } else {
      Serial.println("❌ Not enough memory to load image");
    }
    img.close();
  }

  // 🔚 Xóa task sau khi xong toàn bộ
  vTaskDelete(NULL);
}

void decodeAndSaveImage() {
  File in = SPIFFS.open("/image_base64.txt", FILE_READ);
  if (!in) {
    Serial.println("❌ No base64 file found");
    return;
  }

  File out = SPIFFS.open("/image.jpg", FILE_WRITE);
  if (!out) {
    Serial.println("❌ Cannot open output file");
    in.close();
    return;
  }

  const size_t BLOCK_SIZE = 4096;
  char inBuf[BLOCK_SIZE + 5]; // +5 để thêm \0 và padding an toàn
  unsigned char outBuf[(BLOCK_SIZE / 4) * 3 + 5];
  size_t bytesRead;

  String carry = "";  // Giữ phần dư khi chưa đủ 4 bytes base64

  while ((bytesRead = in.readBytes(inBuf, BLOCK_SIZE)) > 0) {
    inBuf[bytesRead] = '\0';
    String chunk = carry + String(inBuf);
    carry = "";

    // Nếu độ dài không chia hết cho 4, giữ phần dư lại
    int remainder = chunk.length() % 4;
    if (remainder != 0) {
      carry = chunk.substring(chunk.length() - remainder);
      chunk = chunk.substring(0, chunk.length() - remainder);
    }

    size_t decodedLen = 0;
    int ret = mbedtls_base64_decode(outBuf, sizeof(outBuf), &decodedLen,
                                    (unsigned char*)chunk.c_str(), chunk.length());
    if (ret != 0) {
      Serial.printf("⚠️ Decode error at block (len=%d)\n", chunk.length());
      continue;
    }

    out.write(outBuf, decodedLen);
  }

  // Decode phần dư cuối nếu có
  if (carry.length() > 0) {
    size_t decodedLen = 0;
    mbedtls_base64_decode(outBuf, sizeof(outBuf), &decodedLen,
                          (unsigned char*)carry.c_str(), carry.length());
    out.write(outBuf, decodedLen);
  }

  in.close();
  out.close();
  Serial.println("✅ Image decoded and saved → /image.jpg");
}

// -------------------------------------
// WebSocket setup
// -------------------------------------
void initWebSocket() {
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    return;
  }

  webSocket.begin(WS_HOST, WS_PORT, WS_PATH);
  webSocket.setReconnectInterval(5000);

  webSocket.onEvent([](WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        Serial.println("✅ Connected to server");
        webSocket.sendTXT("ESP32 ready!");
        break;

      case WStype_TEXT: {
        if (length < 10 || strncmp((char*)payload, "IMG|", 4) != 0) return;

        String msg = String((char*)payload).substring(4);
        int sep1 = msg.indexOf('|');
        if (sep1 == -1) return;
        String meta = msg.substring(0, sep1);
        String data = msg.substring(sep1 + 1);

        int slash = meta.indexOf('/');
        int part = meta.substring(0, slash).toInt();
        int total = meta.substring(slash + 1).toInt();

        if (part == 1) {
          SPIFFS.remove("/image_base64.txt");
          imgFile = SPIFFS.open("/image_base64.txt", FILE_WRITE);
          if (!imgFile) {
            Serial.println("❌ Cannot create base64 file");
            return;
          }
          Serial.printf("🟢 Receiving image (%d parts)\n", total);
        }

        imgFile.print(data);

if (part == total) {
  imgFile.close();
  Serial.println("✅ Full base64 received");

  // Tạo task để decode dùng heap buffer — tránh stack overflow
  BaseType_t ok = xTaskCreatePinnedToCore(
      decodeTask,       // function
      "decodeTask",     // name
      12288,            // stack size (12 KB) — đủ lớn cho hoạt động
      NULL,             // param
      1,                // priority
      NULL,             // task handle
      1                 // core
  );

  if (ok != pdPASS) {
    Serial.println("❌ Failed to create decode task");
    // Fallback: bạn có thể gọi decodeAndSaveImage() ở đây (không khuyến nghị)
  } else {
    webSocket.sendTXT("IMAGE_OK");
  }

} else {
  Serial.printf("Chunk %d/%d received\n", part, total);
}

        break;
      }

      case WStype_DISCONNECTED:
        Serial.println("🔴 Disconnected from server");
        if (imgFile) imgFile.close();
        break;
    }
  });
}
