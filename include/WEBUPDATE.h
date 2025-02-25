#ifndef WEBUPDATE_H
#define WEBUPDATE_H

#include <Arduino.h>

bool upTools(const String &url, const String &fname, WiFiClientSecure &clientup);
void upFirm();                                              
void updateTools();                                         
void updateSys();                                           
void startHTTPUpdate();                                     
void update_finished();                                     
void update_error(int16_t err);  

#endif