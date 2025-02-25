#ifndef WEB_H
#define WEB_H

#include <Arduino.h>

void handleRoot();                                          
bool loadFromLittlefs(String path);    
void mqttcallback(char *topic, unsigned char *payload, unsigned int length);                     
void handleRequestMisc();
void handleRequestMisc2();     
void handleRequestMiscAlert();
void handleSetMiscLang();    
void handleRequestFirm();       
void handleGetTitle();
void handleReqSys();
void handleSetMisc();
void handleRestore();
void handleGetLanguage();

#endif