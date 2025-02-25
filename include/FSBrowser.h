#ifndef FSBROWSER_H
#define FSBROWSER_H

#include <Arduino.h>

void replyOK();                                             
void replyOKWithMsg(String msg);      
void replyResponse(const char *msg);
void replyReboot(const char *msg);
void replyNotFound(String msg);                             
void replyBadRequest(String msg);                           
void replyServerError(String msg);                          
String getContentType(const String &filename);
void handleStatus();                                        
bool exists(String path);                                   
void handleFileList();                                      
bool handleFileRead(String path);                           
String lastExistingParent(String path);                     
void handleFileCreate();                                    
void deleteRecursive(String path);                          
void handleFileDelete();                                    
void handleFileUpload();         
void handleGetEdit();            

#endif