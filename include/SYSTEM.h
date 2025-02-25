#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <WiFiManager.h> 

void millis2wait(const uint32_t &value);                    
void micros2wait(const uint32_t &value);                    
void setTicker();                                           
void setupTime();
// String decToHex(unsigned char decValue, unsigned char desiredStringLength);
// String IPtoString(IPAddress ip);
bool towCharToHex(const char &MSB, const char &LSB, uint8_t *ptrValue);
// unsigned char convertCharToHex(char ch);        
uint8_t charToHex(char c);            
void sendAlarm(const uint8_t &setAlarm);                    
void EM_LOG();                                              
void EM_REBOOT();                                           
void EM_MDNSET();
void EM_MQTTCONNECT();
void EM_MQTTSUBSCRIBE();
void EM_MQTTRESTORE();
void EM_MQTTERROR();
void debugLog(const String &valFile, const String &valText);
#ifdef ESP32                                          
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);    
#elif ESP8266
void EM_WIFICONNECT(const WiFiEventStationModeGotIP &event);
void EM_WIFIDISCONNECT(const WiFiEventStationModeDisconnected &event);
#endif

#endif