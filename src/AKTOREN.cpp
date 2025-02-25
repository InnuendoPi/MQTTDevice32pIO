#include "MQTTDevice.h"

Actor::Actor(int8_t pin, String argument, String aname, bool ainverted, bool aswitchable)
{
  change(pin, argument, aname, ainverted, aswitchable);
}

void Actor::Update()
{
  if (pin_actor == -100)
    return;
  if (isPin(pin_actor))
  {
    if (isOn && power_actor > 0)
    {
      if (millis() > powerLast + DUTYCYLCE)
      {
        powerLast = millis();
        // DEBUG_VERBOSE("ACT", "powerlast: %lu ms", powerLast);
      }
      if (millis() > powerLast + (DUTYCYLCE * power_actor / 100L))
      {
        digitalWrite(pin_actor, OFF);
#ifdef ESP32
        DEBUG_VERBOSE("ACT", "%s off: %d %\t%s\t%s\t%lu ms", name_actor.c_str(), power_actor, PinToString(pin_actor).c_str(), argument_actor.c_str(), powerLast);
#endif
      }
      else
      {
        digitalWrite(pin_actor, ON);
#ifdef ESP32
        DEBUG_VERBOSE("ACT", "%s on %d %\t%s\t%s\t%lu ms", name_actor.c_str(), power_actor, PinToString(pin_actor).c_str(), argument_actor.c_str(), powerLast);
#endif
      }
    }
    else
    {
      digitalWrite(pin_actor, OFF);
    }
  }
}

void Actor::change(const int8_t &pin, const String &argument, const String &aname, const bool &ainverted, const bool &aswitchable)
{
  if (isPin(pin_actor))
  {
    digitalWrite(pin_actor, OFF);
    pins_used[pin_actor] = false;
    millis2wait(10);
  }

  if (ainverted)
  {
    isInverted = true;
    ON = HIGH;
    OFF = LOW;
  }
  else
  {
    isInverted = false;
    ON = LOW;
    OFF = HIGH;
  }
  pin_actor = pin;

  if (isPin(pin_actor))
  {
    pinMode(pin_actor, OUTPUT);
    digitalWrite(pin_actor, OFF);
    pins_used[pin_actor] = true;
  }

  isOn = false;
  old_isOn = false;
  name_actor = aname;
  if (argument_actor != argument)
  {
    mqtt_unsubscribe();
    argument_actor = argument;
    mqtt_subscribe();
  }

  switchable = aswitchable;
  actor_state = true;
  isOnBeforeError = false;
}

void Actor::mqtt_subscribe()
{
  if (pubsubClient.connected())
  {
    char subscribemsg[50];
    argument_actor.toCharArray(subscribemsg, 50);
#ifdef ESP32
    DEBUG_VERBOSE("ACT", "Subscribing to %s", subscribemsg);
#endif

    pubsubClient.subscribe(subscribemsg);
  }
}

void Actor::mqtt_unsubscribe()
{
  if (pubsubClient.connected())
  {
    char subscribemsg[50];
    argument_actor.toCharArray(subscribemsg, 50);
#ifdef ESP32
    DEBUG_VERBOSE("ACT", "Unsubscribing from %s", subscribemsg);
#endif
    pubsubClient.unsubscribe(subscribemsg);
  }
}

void Actor::handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  JsonDocument filter;
  filter["state"] = true;
  filter["power"] = true;
  DeserializationError error = deserializeJson(doc, (const char *)payload, DeserializationOption::Filter(filter));
  if (error)
  {
#ifdef ESP32
    DEBUG_ERROR("ACT", "handlemqtt deserialize Json error %s", error.c_str());
#endif
    return;
  }
  if (doc["state"] == "off")
  {
    isOn = false;
    power_actor = 0;
    return;
  }
  if (doc["state"] == "on")
  {
    isOn = true;
    int16_t newPower = doc["power"];
    power_actor = constrain(newPower, 0, 100);
    return;
  }
}

String Actor::getActorName()
{
  return name_actor;
}
String Actor::getActorTopic()
{
  return argument_actor;
}
bool Actor::getInverted()
{
  return isInverted;
}
bool Actor::getActorSwitch()
{
  return switchable;
}
bool Actor::getIsOn()
{
  return isOn;
}
void Actor::setIsOn(const bool &val)
{
  isOn = val;
}
bool Actor::getOldIsOn()
{
  return old_isOn;
}
bool Actor::getActorState()
{
  return actor_state;
}
void Actor::setActorState(const bool &val)
{
  actor_state = val;
}
bool Actor::getIsOnBeforeError()
{
  return isOnBeforeError;
}
void Actor::setIsOnBeforeError(const bool &val)
{
  isOnBeforeError = val;
}
int8_t Actor::getPinActor()
{
  return pin_actor;
}
uint8_t Actor::getActorPower()
{
  return power_actor;
}
void Actor::setOldIsOn()
{
  old_isOn = isOn;
}

// Funktionen für Loop im Timer Objekt
void handleActors(bool checkAct)
{
  // checkAct true: init
  // checkAct false: only updates

  JsonDocument ssedoc;
  // JsonArray sseArray = ssedoc.to<JsonArray>();
  for (uint8_t i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    if (actors[i].getOldIsOn() != actors[i].getIsOn())
    {
      actors[i].setOldIsOn();
      checkAct = true;
    }
    JsonObject sseObj = ssedoc.add<JsonObject>();
    sseObj["name"] = actors[i].getActorName();
    sseObj["ison"] = actors[i].getIsOn();
    sseObj["power"] = actors[i].getActorPower();
    sseObj["mqtt"] = actors[i].getActorTopic();
    sseObj["state"] = actors[i].getActorState();
    sseObj["pin"] = PinToString(actors[i].getPinActor());
    yield();
  }

  if (checkAct)
  {
    char response[measureJson(ssedoc) + 1];
    serializeJson(ssedoc, response, sizeof(response));
    SSEBroadcastJson(response, ACTORJSON);
  }
}

/* Funktionen für Web */
void handleRequestActors()
{
  int8_t id = server.arg(0).toInt();
  JsonDocument doc;
  if (id == -1) // fetch all sensors
  {
    // JsonArray actorsArray = doc.to<JsonArray>();

    for (uint8_t i = 0; i < numberOfActors; i++)
    {
      JsonObject actorsObj = doc.add<JsonObject>();
      actorsObj["name"] = actors[i].getActorName();
      actorsObj["ison"] = actors[i].getIsOn();
      actorsObj["power"] = actors[i].getActorPower();
      actorsObj["mqtt"] = actors[i].getActorTopic();
      actorsObj["pin"] = actors[i].getPinActor();
      actorsObj["sw"] = actors[i].getActorSwitch();
      actorsObj["state"] = actors[i].getActorState();
      yield();
    }
  }
  else
  {
    doc["name"] = actors[id].getActorName();
    doc["mqtt"] = actors[id].getActorTopic();
    doc["sw"] = actors[id].getActorSwitch();
    doc["inv"] = actors[id].getInverted();
  }

  char response[measureJson(doc) + 1];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void handleSetActor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
#ifdef ESP32
    DEBUG_ERROR("ACT", "error deserializeJson %s", error.c_str());
#endif
    replyServerError("Server error deserialize actor json");
    return;
  }
  int8_t id = doc["id"];
  if (id == -1) // new actor
  {
    if (numberOfActors >= NUMBEROFACTORSMAX)
    {
#ifdef ESP32
      DEBUG_ERROR("ACT", "error max number actors");
#endif
      replyServerError("Server error max number actors");
      return;
    }
    id = numberOfActors;
    numberOfActors++;
  }
  actors[id].change(StringToPin(doc["pin"]), doc["mqtt"], doc["name"], doc["inv"], doc["sw"]);
  server.send(200);
  saveConfig();
  handleActors(true);
  TickerAct.setLastTime(millis());
}

void handleDelActor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
#ifdef ESP32
    DEBUG_ERROR("ACT", "error deserializeJson %s", error.c_str());
#endif
    replyServerError("Server error delete actor");
    return;
  }
  int8_t id = doc["id"];
  if (id < 0 || id > numberOfActors)
  {
#ifdef ESP32
    DEBUG_ERROR("ACT", "error delete actor out of bounds");
#endif
    replyServerError("Server error delete actor out of bounds");
    return;
  }
  actors[id].setIsOn(false);
  pins_used[actors[id].getPinActor()] = false;

  for (uint8_t i = id; i < numberOfActors; i++)
  {
    if (i == (NUMBEROFACTORSMAX - 1)) // 5 - Array von 0 bis (NUMBEROFACTORSMAX-1)
    {
      actors[i].change(-100, "", "", false, false);
    }
    else
    {
      actors[i].change(actors[i + 1].getPinActor(), actors[i + 1].getActorTopic(), actors[i + 1].getActorName(), actors[i + 1].getInverted(), actors[i + 1].getActorSwitch());
    }
    yield();
  }

  if (numberOfActors > 0)
    numberOfActors--;
  else
    numberOfActors = 0;
  server.send(200);
  saveConfig();
  handleActors(true);
  TickerAct.setLastTime(millis());
}

void handlereqPins()
{
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += OPTIONSTART;
    message += PinToString(actors[id].getPinActor());
    message += OPTIONDISABLED;
  }
  for (int i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += OPTIONSTART;
      message += pin_names[i];
      message += OPTIONEND;
    }
    yield();
  }
  replyOKWithMsg(message.c_str());
}

int8_t StringToPin(const String &pinstring)
{
  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pin_names[i] == pinstring)
      return pins[i];
  }
  return -100;
}

String PinToString(const int8_t &pinbyte)
{
  for (int i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins[i] == pinbyte)
      return pin_names[i];
  }
  return "-100";
}

bool isPin(const int8_t &pinbyte)
{
  if (pinbyte == -100)
    return false;

  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins[i] == pinbyte)
    {
      return true;
    }
  }
  return false;
}

void actERR()
{
  for (uint8_t i = 0; i < numberOfActors; i++)
  {
    if (actors[i].getActorSwitch() && actors[i].getActorState() && actors[i].getIsOn())
    {
      actors[i].setIsOnBeforeError(actors[i].getIsOn());
      actors[i].setIsOn(false);
      actors[i].setActorState(false);
      actors[i].Update();
#ifdef ESP32
      DEBUG_ERROR("ACT", "ACT MQTT event handling - actor: %s state: %d isOnBeforeError: %d", actors[i].getActorName().c_str(), actors[i].getActorState(), actors[i].getIsOnBeforeError());
#endif
    }
    yield();
  }
}