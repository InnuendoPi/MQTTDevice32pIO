#ifndef SSE_H
#define SSE_H

#include <Arduino.h>

void startSSE();
void inductionSSE(bool val);
void miscSSE();
void handleChannel();
void SSEKeepAlive();
void SSEHandler(const uint8_t &channel);
void SSEBroadcastJson(const char *jsonValue, uint8_t typ);
void handleNotFound();
void handleAll();
void checkAliveSSE();
void initialSSE(const uint8_t &val); 

#endif