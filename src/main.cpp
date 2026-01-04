#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "display_handler.h"
#include "websocket_client.h"
#include "mbedtls/base64.h"

// WiFi config
const char* SSID = "P419/3";
const char* PASSWORD = "12345678@@";

// WebSocket config
const char* websocket_server = "192.168.1.13";
const uint16_t websocket_port = 8080;
const char* websocket_path = "/ws/iot?clientType=esp32";

// TFT Display
Adafruit_ST7735 tft = Adafruit_ST7735(5, 2, 23, 18, 4);
WebSocketsClient webSocket;

// Buffers
uint8_t jpegBuffer[65536];
int jpegBufferLen = 0;

String chunks[10];
unsigned long lastChunkTime = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 WebSocket Display ===");

    // ===== TFT TEST =====
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);

    tft.fillScreen(ST77XX_RED); delay(300);
    tft.fillScreen(ST77XX_GREEN); delay(300);
    tft.fillScreen(ST77XX_BLACK);

    tft.setCursor(0, 0);
    tft.println("Connecting WiFi...");

    // ===== WiFi =====
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.println("WiFi OK");

    // ===== Init WebSocket =====
    initWebSocket(websocket_server, websocket_port, websocket_path);

    tft.println("WebSocket Ready");
}

void loop() {
    webSocket.loop();

    // Chunk timeout reset
    if (millis() - lastChunkTime > 10000) {
        for (int i = 1; i < 10; i++) chunks[i] = "";
        lastChunkTime = millis();
    }

    delay(10);
}
