#include "config.h"
#include "mbedtls/base64.h"

const char* WIFI_SSID = "P419/3";
const char* WIFI_PASS = "12345678@@";
const char* WS_HOST   = "192.168.1.3";
const int   WS_PORT   = 8080;   // 👈 Thêm dòng này
const char* WS_PATH   = "/ws/iot";

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
