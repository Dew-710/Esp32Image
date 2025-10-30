#include <Arduino.h>
#include "config.h"
#include "websocket_client.h"
#include "display_handler.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initWebSocket();
  initDisplay();
}

void loop() {
  webSocket.loop();
}
