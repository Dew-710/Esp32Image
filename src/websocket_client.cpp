#include "websocket_client.h"
#include "display_handler.h"
#include "mbedtls/base64.h"
#include <Arduino.h>

extern uint8_t jpegBuffer[];
extern int jpegBufferLen;
extern String chunks[];
extern unsigned long lastChunkTime;

extern WebSocketsClient webSocket;

static const int MAX_CHUNKS = 9;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch (type) {

        case WStype_DISCONNECTED:
            Serial.println("[WS] ❌ Disconnected");
            break;

        case WStype_CONNECTED:
            Serial.printf("[WS] ✅ Connected: %s\n", payload);
            lastChunkTime = millis();
            break;

        case WStype_ERROR:
            Serial.printf("[WS] ❌ ERROR: %s\n", payload);
            break;

        case WStype_TEXT: {
            String msg = (char*) payload;

            if (!msg.startsWith("IMG|")) {
                Serial.println("[WS] Unknown text message");
                return;
            }

            int p1 = msg.indexOf('|');
            int p2 = msg.indexOf('|', p1 + 1);
            if (p1 < 0 || p2 < 0) return;

            String chunkInfo = msg.substring(p1 + 1, p2); // "1/5"
            String chunkBase64 = msg.substring(p2 + 1);

            int slash = chunkInfo.indexOf('/');
            int idx = chunkInfo.substring(0, slash).toInt();
            int total = chunkInfo.substring(slash + 1).toInt();

            if (idx < 1 || idx > MAX_CHUNKS || total > MAX_CHUNKS) {
                Serial.println("[WS] Invalid chunk index");
                return;
            }

            chunks[idx] = chunkBase64;
            Serial.printf("[WS] Chunk %d/%d OK (%d bytes)\n",
                          idx, total, chunkBase64.length());

            lastChunkTime = millis();

            bool complete = true;
            for (int i = 1; i <= total; i++) {
                if (chunks[i].isEmpty()) complete = false;
            }

            if (!complete) return;

            String fullBase64 = "";
            for (int i = 1; i <= total; i++) fullBase64 += chunks[i];
            for (int i = 1; i < 10; i++) chunks[i] = "";

            if (fullBase64.startsWith("data:image/jpeg;base64,"))
                fullBase64 = fullBase64.substring(23);

            size_t outLen = 0;
            int ret = mbedtls_base64_decode(
                jpegBuffer, 65536, &outLen,
                (const unsigned char*) fullBase64.c_str(),
                fullBase64.length()
            );

            if (ret != 0 || outLen == 0) {
                Serial.println("[WS] ❌ Base64 decode FAILED");
                return;
            }

            jpegBufferLen = outLen;
            Serial.printf("[WS] JPEG size: %u bytes\n", jpegBufferLen);

            decodeAndDisplayJPEG(jpegBuffer, jpegBufferLen);
        }
        break;

        default:
            break;
    }
}

void initWebSocket(const char* server, uint16_t port, const char* path) {

    Serial.println("\n=== WebSocket Init ===");

    // CRITICAL FIX: Use standard WebSocket protocol (not "arduino")
    webSocket.begin(server, port, path, "");  // Empty string = no subprotocol
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    webSocket.enableHeartbeat(15000, 3000, 2);

    Serial.println("[WS] Ready - Standard Protocol");
}
