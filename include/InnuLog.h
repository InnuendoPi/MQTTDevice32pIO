#ifndef INNULOG_H
#define INNULOG_H

#include <Arduino.h>

#ifdef ESP32
int getTagLevel(const String &tagName);                     
void setTagLevel(const String &tagName, int level);         
void saveLog();                                             
void readLog();                                            
#endif

#endif