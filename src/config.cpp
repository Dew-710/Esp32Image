#include "config.h"
#include "mbedtls/base64.h"

const char* WIFI_SSID = "P419/3";
const char* WIFI_PASS = "12345678@@";
const char* WS_HOST   = "192.168.1.9";
const int   WS_PORT   = 8080;
const char* WS_PATH   = "/ws/iot?clientType=esp32";  // ⚠️ QUAN TRỌNG: Phải có query parameter

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("🔌 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
