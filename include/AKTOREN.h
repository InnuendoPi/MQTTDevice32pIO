#ifndef ACTOREN_H
#define ACTOREN_H

#include <Arduino.h>

class Actor
{
private:
unsigned long powerLast; // Zeitmessung für High oder Low
  unsigned char OFF;
  unsigned char ON;
  int8_t pin_actor = -100; // the number of the LED pin
  String argument_actor;
  String name_actor;
  uint8_t power_actor;
  bool isInverted = false;
  bool switchable;              // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true;      // Error state actor
  bool isOn;
  bool old_isOn;

public:
    Actor(int8_t, String, String, bool, bool);

    void Update();
    void change(const int8_t &pin, const String &argument, const String &aname, const bool &ainverted, const bool &aswitchable);
    // void handlePWM(const int16_t &newPower);
    void mqtt_subscribe();
    void mqtt_unsubscribe();
    void handlemqtt(unsigned char *payload);
    String getActorName();
    String getActorTopic();
    bool getInverted();
    bool getActorSwitch();
    bool getIsOn();
    void setIsOn(const bool &val);
    bool getOldIsOn();
    bool getActorState();
    void setActorState(const bool &val);
    bool getIsOnBeforeError();
    void setIsOnBeforeError(const bool &val);
    int8_t getPinActor();
    uint8_t getActorPower();
    void setOldIsOn();
};

void handleActors(bool checkAct);                           
void handleRequestActors();  
void handleSetActor(); 
void handleDelActor();                                      
void handlereqPins();                                       
int8_t StringToPin(const String &pinstring);                
String PinToString(const int8_t &pinbyte);
bool isPin(const int8_t &pinbyte);
void actERR();

#endif