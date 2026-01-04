#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <WebSocketsClient.h>

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void initWebSocket(const char* server, uint16_t port, const char* path);

#endif
