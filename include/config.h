#ifndef CONFIG_H
#define CONFIG_H

#include <WiFi.h>

// Khai báo extern - chỉ là "nói trước" rằng biến này sẽ được định nghĩa ở nơi khác
extern const char* WIFI_SSID;
extern const char* WIFI_PASS;
extern const char* WS_HOST;
extern const char* WS_PATH;
extern const int   WS_PORT;

// Khai báo hàm để có thể dùng ở file khác
void connectToWiFi();

#endif
