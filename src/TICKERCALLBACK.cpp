#include "MQTTDevice.h"

void tickerDispCallback()
{
  if (nextion.currentPageId != nextion.lastCurrentPageId)
  {
    activePage = nextion.currentPageId;
    nextion.lastCurrentPageId = nextion.currentPageId;
  }

  char ipMQTT[50];
  
  // sprintf_P(uhrzeit, (PGM_P)F("%02d:%02d"), timeinfo.tm_hour, timeinfo.tm_min);
  snprintf(uhrzeit, sizeof(uhrzeit), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  if (startMDNS)
    // sprintf_P(ipMQTT, (PGM_P)F("http://%s.local"), nameMDNS);
    snprintf(ipMQTT, sizeof(ipMQTT), "http://%s.local", nameMDNS);
  else
    // sprintf_P(ipMQTT, (PGM_P)F("http://%s"), WiFi.localIP().toString().c_str());
    snprintf(ipMQTT, sizeof(ipMQTT), "http://%s", WiFi.localIP().toString().c_str());

  activePage = nextion.currentPageId;
  switch (activePage)
  {
  case 0: // KettlePage
    nextion.writeStr(uhrzeit_text, uhrzeit);
    nextion.writeStr(mqttDevice, ipMQTT);
    KettlePage();
    break;
  case 1: // BrewPage
    if (strlen(structKettles[0].sensor) == 0)
      strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);

    nextion.writeStr(p1mqttDevice, ipMQTT);
    nextion.writeStr(p1uhrzeit_text, uhrzeit);
    BrewPage();
    break;
  case 2: // Induction mode
    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);
    nextion.writeStr(p2uhrzeit_text, uhrzeit);
    InductionPage();
    break;
  }
}

void tickerPUBSUBCallback() // Timer Objekt Sensoren
{
  if (pubsubClient.connected())
  {
    mqtt_state = true;
    pubsubClient.loop();
    if (TickerMQTT.state() == RUNNING)
      TickerMQTT.stop();

    return;
  }
  else
  {
    if (TickerMQTT.state() != RUNNING)
    {
      #ifdef ESP32
      DEBUG_ERROR("SYS", "Ticker PubSub Error: TickerMQTT started");
      #endif
      TickerMQTT.start();
      mqttconnectlasttry = millis();
      mqtt_state = false; // MQTT in error state
      miscSSE();
    }
    TickerMQTT.update();
  }
}

void tickerSenCallback() // Timer Objekt Sensoren
{
  DS18B20.requestTemperatures();
  // lastRequestSensors = millis();
  if (StopOnMQTTError) // Event handling enabled?
  {
    switch (sensorsStatus)
    {
    case EM_OK:
      // all sensors ok
      lastSenInd = 0; // Delete induction timestamp after event
      lastSenAct = 0; // Delete actor timestamp after event
      // if (WiFi.status() == WL_CONNECTED && mqtt_state)
      if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
      {
        for (int i = 0; i < numberOfActors; i++)
        {
          if (actors[i].getActorSwitch() && !actors[i].getActorState()) // Sensor in normal mode: check actor in error state
          {
            #ifdef ESP32
            DEBUG_VERBOSE("SEN", "EM SenOK: %s isOnBeforeError: %d power level: %d", actors[i].getActorName().c_str(), actors[i].getIsOnBeforeError(), actors[i].getActorPower());
            #endif
            actors[i].setIsOn(actors[i].getIsOnBeforeError());
            actors[i].setActorState(true);
            actors[i].Update();
            lastSenAct = 0; // Delete actor timestamp after event
          }
          yield();
        }

        if (!inductionCooker.getInductionState())
        {
          #ifdef ESP32
          DEBUG_VERBOSE("SEN", "EM SenOK: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d", inductionCooker.getPower(), inductionCooker.getPowerLevelOnError(), inductionCooker.getPowerLevelBeforeError());
          #endif
          if (!inductionCooker.getInductionState())
          {
            inductionCooker.setNewPower(inductionCooker.getPowerLevelBeforeError());
            inductionCooker.setisInduon(true);
            inductionCooker.setInductionState(true);
            inductionCooker.Update();
            lastSenInd = 0; // Delete induction timestamp after event
          }
        }
      }
      break;
    case EM_CRCER:
      // Sensor CRC ceck failed
    case EM_DEVER:
      // -127°C device error
    case EM_UNPL:
      // sensor unpluged
    case EM_SENER:
      // all other errors
      // if (WiFi.status() == WL_CONNECTED && mqtt_state)
      if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
      {
        for (int i = 0; i < numberOfSensors; i++)
        {
          if (!sensors[i].getSensorState() && ( lastSenInd == 0 || lastSenAct == 0 ))
          {
            switch (sensorsStatus)
            {
            case EM_CRCER:
              // Sensor CRC ceck failed
              #ifdef ESP32
              DEBUG_ERROR("SEN", "EM CRCER: Sensor %s crc check failed", sensors[i].getSensorName().c_str());
              #endif
              break;
            case EM_DEVER:
              // -127°C device error
              #ifdef ESP32
              DEBUG_ERROR("SEN", "EM DEVER: Sensor %s device error", sensors[i].getSensorName().c_str());
              #endif
              break;
            case EM_UNPL:
              // sensor unplugged
              #ifdef ESP32
              DEBUG_ERROR("SEN", "EM UNPL: Sensor %s unplugged", sensors[i].getSensorName().c_str());
              #endif
              break;
            default:
              break;
            }
          }

          if (sensors[i].getSensorSwitch() && !sensors[i].getSensorState())
          {
            if (lastSenAct == 0)
            {
              lastSenAct = millis(); // Timestamp on error
              #ifdef ESP32
              DEBUG_VERBOSE("ACT", "timestamp actors due to sensor error: %ul Wait on error actors: %ul", lastSenAct, wait_on_Sensor_error_actor / 1000);
              #endif
            }
            if (lastSenInd == 0)
            {
              lastSenInd = millis(); // Timestamp on error
              #ifdef ESP32
              DEBUG_VERBOSE("IND", "timestamp induction due to sensor error: %ul Wait on error induction: %ul", lastSenInd, wait_on_Sensor_error_induction / 1000);
              #endif
            }
            if (millis() - lastSenAct >= wait_on_Sensor_error_actor) // Wait bevor Event handling
            {
              actERR();
            }
            if (millis() - lastSenInd >= wait_on_Sensor_error_induction) // Wait bevor Event handling
            {
              if (inductionCooker.getisInduon() && inductionCooker.getPowerLevelOnError() < 100 && inductionCooker.getInductionState())
              {
                inductionCooker.indERR();
              }
            }
          } // Switchable
          yield();
        } // Iterate sensors
      } // wlan und mqtt state
      break;
    default:
      break;
    }
  }
  handleSensors(false);
}

void tickerActCallback() // Timer Objekt Sensoren
{
  handleActors(false);
}

void tickerIndCallback() // Timer Objekt Sensoren
{
  handleInduction();
  inductionSSE(false);
}

void tickerMQTTCallback() // Ticker helper function calling Event MQTT Error
{
  if (TickerMQTT.counter() == 1)
  {
    switch (pubsubClient.state())
    {
    case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECTION_TIMEOUT", pubsubClient.state());
      #endif
      break;
    case -3: // MQTT_CONNECTION_LOST - the network connection was broken
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECTION_LOST", pubsubClient.state());
      #endif
      break;
    case -2: // MQTT_CONNECT_FAILED - the network connection failed
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_FAILED", pubsubClient.state());
      #endif
      break;
    case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_DISCONNECTED", pubsubClient.state());
      #endif
      break;
    case 0: // MQTT_CONNECTED - the client is connected
      pubsubClient.loop();
      break;
    case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_BAD_PROTOCOL", pubsubClient.state());
      #endif
      break;
    case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_BAD_CLIENT_ID", pubsubClient.state());
      #endif
      break;
    case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_UNAVAILABLE", pubsubClient.state());
      #endif
      break;
    case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_BAD_CREDENTIALS", pubsubClient.state());
      #endif
      break;
    case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
    #ifdef ESP32
      DEBUG_ERROR("SYS", "MQTT status: error rc=%d MQTT_CONNECT_UNAUTHORIZED", pubsubClient.state());
      #endif
      break;
    default:
      break;
    }
  }
  EM_MQTTERROR();
}

void tickerTimeCallback()
{
  getLocalTime(&timeinfo);
  strftime(zeit, sizeof(zeit), "%H:%M:%S", &timeinfo);
  // DEBUG_VERBOSE("SYS", "SNTP time: %s", zeit);
  if (useDisplay)
    tickerDispCallback();
}