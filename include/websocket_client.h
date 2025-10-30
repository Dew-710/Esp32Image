#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <WebSocketsClient.h>

extern WebSocketsClient webSocket;

void initWebSocket();
void sendBase64Image(String base64Image);
void decodeAndSaveImage();

#endif
