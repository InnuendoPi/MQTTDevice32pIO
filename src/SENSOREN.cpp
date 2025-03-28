#include "MQTTDevice.h"

TemperatureSensor::TemperatureSensor(String new_address, String new_mqtttopic, String new_name, String new_id, float new_offset1, float new_offset2, bool new_sw, uint8_t new_type, uint8_t new_pin)
{
  change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw, new_type, new_pin);
}

void TemperatureSensor::Update()
{
  sens_value = DS18B20.getTempC(sens_address);
  sensorsStatus = 0;
  sens_state = true;
  if (simErr == 1) // simulate sensor error http://mqttdevice.local/setSenErr?err=1
  {
    sens_value = -127.0;
  }
  if (sens_type == 0)
  {
    if (sens_value <= DEVICE_DISCONNECTED_C)
    {
      sensorsStatus = EM_SENER;
#ifdef ESP32
      if (old_state)
        DEBUG_ERROR("SEN", "sensor error");
#endif
      sens_state = false;
      old_state = sens_state;
    }
    else
    {
      sensorsStatus = EM_OK;
      sens_state = true;
    }
    sens_err = sensorsStatus;
  } // if senstyp DS18B20
  else if (sens_type == 1)
  {
    sensorsStatus = 0;
    sens_state = true;

    if (sens_ptid == 0)
      sens_value = pt_0.temperature(RNOMINAL100, RREF100);
    else if (sens_ptid == 1)
      sens_value = pt_1.temperature(RNOMINAL100, RREF100);
    else if (sens_ptid == 2)
      sens_value = pt_2.temperature(RNOMINAL100, RREF100);

    if (sens_value > 120.0 || sens_value < -100.0) // außerhalb realistischer Messbereich
    {
      sensorsStatus = EM_SENER;
#ifdef ESP32
      if (old_state)
        DEBUG_ERROR("SEN", "PT100 sen error");
#endif
      sens_state = false;
      old_state = sens_state;
    }
    sens_err = sensorsStatus;
  } // if senstyp pt100
  else if (sens_type == 2)
  {
    sensorsStatus = 0;
    sens_state = true;
    if (sens_ptid == 0)
      sens_value = pt_0.temperature(RNOMINAL1000, RREF1000);
    else if (sens_ptid == 1)
      sens_value = pt_1.temperature(RNOMINAL1000, RREF1000);
    else if (sens_ptid == 2)
      sens_value = pt_2.temperature(RNOMINAL1000, RREF1000);

    if (sens_value > 120.0 || sens_value < -100.0) // außerhalb realistischer Messbereich
    {
      sensorsStatus = EM_SENER;
#ifdef ESP32
      if (old_state)
        DEBUG_ERROR("SEN", "PT1000 sen error");
#endif
      sens_state = false;
      old_state = sens_state;
    }
    sens_err = sensorsStatus;
  } // if senstyp pt1000
#ifdef ESP32
  DEBUG_VERBOSE("SEN", "%s\t%s\t%s\t%g\t%g/%g", sens_name.c_str(), sens_type == 0 ? "DS18B20" : "PT100x", sens_mqtttopic, sens_value, sens_offset1, sens_offset2);
#endif
  if (TickerPUBSUB.state() == RUNNING && TickerMQTT.state() != RUNNING)
    publishmqtt();
} // void Update

void TemperatureSensor::change(const String &new_address, const String &new_mqtttopic, const String &new_name, const String &new_id, float new_offset1, float new_offset2, const bool &new_sw, const uint8_t &new_type, const uint8_t &new_pin)
{
  new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
  // strlcpy(sens_mqtttopic, new_mqtttopic.c_str(), 49);
  // sprintf(sens_mqtttopic, "%s", new_mqtttopic.c_str());
  sens_id = new_id;
  sens_name = new_name;
  sens_offset1 = new_offset1;
  sens_offset2 = new_offset2;
  sens_sw = new_sw;
  sens_type = new_type;
  sens_pin = new_pin;

  if (sens_type == 0)
  {
    sens_ptid = -1;
    if (new_address.length() == 16)
    {
      for (size_t i = 0; i < 8; i++)
      {
        char cMSB = new_address[2 * i];
        char cLSB = new_address[2 * i + 1];
        uint8_t tmpValue = 0;
        if (towCharToHex(cMSB, cLSB, &tmpValue))
          sens_address[i] = tmpValue;
      }
    }
    if (senRes)
    {
      DS18B20.setResolution(sens_address, RESOLUTION_HIGH);
#ifdef ESP32
      DEBUG_INFO("SEN", "senor: %s set resolution high", sens_name.c_str());
#endif
    }
    else
    {
      DS18B20.setResolution(sens_address, RESOLUTION);
#ifdef ESP32
      DEBUG_INFO("SEN", "senor: %s set resolution normal", sens_name.c_str());
#endif
    }
  }
}

void TemperatureSensor::publishmqtt()
{
  if (pubsubClient.connected())
  {
    JsonDocument doc;
    JsonObject sensorsObj = doc["Sensor"].to<JsonObject>();
    sensorsObj["Name"] = sens_name;
    if (sensorsStatus == 0)
    {
      sensorsObj["Value"] = calcOffset();
    }
    else
    {
      sensorsObj["Value"] = sens_value;
    }
    if (sens_type == 1)
      sensorsObj["Type"] = "PT100";
    else if (sens_type == 2)
      sensorsObj["Type"] = "PT1000";
    else
      sensorsObj["Type"] = "1-wire";
    char jsonMessage[100];
    serializeJson(doc, jsonMessage);
    pubsubClient.publish(sens_mqtttopic, jsonMessage);
  }
}
int8_t TemperatureSensor::getErr()
{
  return sens_err;
}
bool TemperatureSensor::getSensorSwitch()
{
  return sens_sw;
}
bool TemperatureSensor::getSensorState()
{
  return sens_state;
}
float TemperatureSensor::getOffset1()
{
  return sens_offset1;
}
float TemperatureSensor::getOffset2()
{
  return sens_offset2;
}
float TemperatureSensor::getValue()
{
  return sens_value;
}
float TemperatureSensor::oldValue()
{
  return old_value;
}
void TemperatureSensor::setOldValue()
{
  old_value = sens_value;
}
String TemperatureSensor::getSensorName()
{
  return sens_name;
}
String TemperatureSensor::getSensorTopic()
{
  return sens_mqtttopic;
}
String TemperatureSensor::getId()
{
  return sens_id;
}
char *TemperatureSensor::getValueString()
{
  dtostrf(sens_value, -1, 1, buf);
  return buf;
}

float TemperatureSensor::getTotalValueFloat()
{
  return round((calcOffset() - 0.05) * 10) / 10.0;
}

char *TemperatureSensor::getTotalValueString()
{
  if (sens_value == -127.00)
    snprintf(buf, sizeof(buf), "%s", "-127.0");
  else
  {
    snprintf(buf, sizeof(buf), "%s", "0.0");
    dtostrf((round((calcOffset() - 0.04) * 10) / 10.0), -1, 1, buf);
  }
  return buf;
}

float TemperatureSensor::calcOffset()
{
  if (sens_value == -127.00)
    return sens_value;
  if (sens_offset1 == 0.0 && sens_offset2 == 0.0) // keine Kalibrierung
  {
    return sens_value;
  }
  else if ((sens_offset1 != 0.0 && sens_offset2 != 0.0) || (sens_offset1 == 0.0 && sens_offset2 != 0.0)) // 2-Punkte-Kalibrierung
  {
    float m = (TEMP_OFFSET2 - TEMP_OFFSET1) / ((TEMP_OFFSET2 + sens_offset2) - (TEMP_OFFSET1 + sens_offset1));
    float b = ((TEMP_OFFSET2 + sens_offset2) * TEMP_OFFSET1 - ((TEMP_OFFSET1 + sens_offset1) * TEMP_OFFSET2)) / ((TEMP_OFFSET2 + sens_offset2) - (TEMP_OFFSET1 + sens_offset1));
    return m * sens_value + b;
  }
  else if (sens_offset1 != 0.0 && sens_offset2 == 0.0) // 1-Punkt-Kalibrierung
  {
    return sens_value + sens_offset1;
  }
  return sens_value;
}

String TemperatureSensor::getSens_adress_string()
{
  return SensorAddressToString(sens_address);
}
uint8_t TemperatureSensor::getSensType()
{
  return sens_type;
}
void TemperatureSensor::setSensType(const uint8_t &val)
{
  sens_type = constrain(val, 0, 2);
}
uint8_t TemperatureSensor::getSensPin()
{
  return sens_pin;
}
void TemperatureSensor::setSensPTid(const int8_t &val)
{
  sens_ptid = val;
}

void handleSensors(bool checkSen)
{
  // checkSen true: init
  // checkSen false: only updates

  JsonDocument ssedoc;
  JsonArray sseArray = ssedoc.to<JsonArray>();
  int8_t max_status = 0;
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    sensors[i].Update();

    if (sensors[i].getSensorSwitch() && max_status < sensors[i].getErr())
      max_status = sensors[i].getErr();
    if (sensors[i].getValue() != sensors[i].oldValue())
    {
      sensors[i].setOldValue();
      checkSen = true;
    }
    JsonObject sseObj = ssedoc.add<JsonObject>();
    sseObj["name"] = sensors[i].getSensorName();
    sseObj["typ"] = sensors[i].getSensType();
    if (sensors[i].getErr() == EM_OK)
      sseObj["value"] = sensors[i].getTotalValueString();
    else
      sseObj["value"] = "ERR";

    yield();
  }
  sensorsStatus = max_status;
  if (checkSen)
  {
    char response[measureJson(ssedoc) + 1];
    serializeJson(ssedoc, response, sizeof(response));
    SSEBroadcastJson(response, SENSORJSON);
  }
}

String SensorAddressToString(uint8_t deviceAddress[8])
{
  String addressString = "";
  for (size_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      addressString += "0";
    addressString += String(deviceAddress[i], HEX);
  }
  return addressString;
}

void handleSetSensor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
#ifdef ESP32
    DEBUG_ERROR("SEN", "error deserializeJson %s", error.c_str());
#endif
    replyServerError("Server error deserialize sensor json");
    return;
  }
  int8_t id = doc["id"];
  if (id == -1) // new sensor
  {
    if (numberOfSensors >= NUMBEROFSENSORSMAX)
    {
      replyServerError("Server error max number sensors");
      return;
    }
    id = numberOfSensors;
    numberOfSensors++;
  }

  sensors[id].change(doc["address"], doc["script"], doc["name"], doc["cbpiid"], doc["offset1"], doc["offset2"], doc["sw"], doc["type"], doc["pin"]);
  server.send(200);
  saveConfig();
  setupPT();
  handleSensors(true);
  TickerSen.setLastTime(millis()); // requiered for async mode
}

void handleDelSensor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
#ifdef ESP32
    DEBUG_ERROR("SEN", "error deserializeJson %s", error.c_str());
#endif
    replyServerError("Server error sensor delete");
    return;
  }
  int8_t id = doc["id"];
  for (uint8_t i = id; i < numberOfSensors; i++)
  {
    if (i == (NUMBEROFSENSORSMAX - 1)) // 5 - Array von 0 bis (NUMBEROFSENSORSMAX-1)
    {
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].getSensorTopic(), sensors[i + 1].getSensorName(), sensors[i + 1].getId(), sensors[i + 1].getOffset1(), sensors[i + 1].getOffset2(), sensors[i + 1].getSensorSwitch(), sensors[i + 1].getSensType(), sensors[i + 1].getSensPin());

    yield();
  }
  if (numberOfSensors > 0)
    numberOfSensors--;
  else
    numberOfSensors = 0;
  server.send(200);
  saveConfig();
  handleSensors(true);
  TickerSen.setLastTime(millis());
}

void handleRequestSensorAddresses()
{
  uint8_t numberOfSensorsFound = DS18B20.getDeviceCount();
  if (numberOfSensorsFound > NUMBEROFSENSORSMAX)
    numberOfSensorsFound = NUMBEROFSENSORSMAX;

  int8_t id = server.arg(0).toInt();
  String message;
  if (id != -1)
  {
    message += OPTIONSTART;
    message += sensors[id].getSens_adress_string();
    message += OPTIONDISABLED;
  }
  for (uint8_t i = 0; i < numberOfSensorsFound; i++)
  {
    DS18B20.getAddress(addressesFound[i], i);
    message += OPTIONSTART;
    message += SensorAddressToString(addressesFound[i]);
    message += OPTIONEND;
    yield();
  }
  replyOKWithMsg(message.c_str());
}

void handleRequestSensors()
{
  int8_t id = server.arg(0).toInt();
  JsonDocument doc;
  if (id == -1) // fetch all sensors
  {
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      JsonObject sensorsObj = doc.add<JsonObject>();
      sensorsObj["name"] = sensors[i].getSensorName();
      sensorsObj["type"] = sensors[i].getSensType();
      sensorsObj["offset1"] = sensors[i].getOffset1();
      sensorsObj["offset2"] = sensors[i].getOffset2();
      sensorsObj["sw"] = sensors[i].getSensorSwitch();
      sensorsObj["state"] = sensors[i].getSensorState();

      if (sensors[i].getErr() == EM_OK)
        sensorsObj["value"] = sensors[i].getTotalValueString();
      else if (sensors[i].getErr() == EM_CRCER)
        sensorsObj["value"] = "CRC";
      else if (sensors[i].getErr() == EM_DEVER)
        sensorsObj["value"] = "DER";
      else if (sensors[i].getErr() == EM_UNPL)
        sensorsObj["value"] = "UNP";
      else
        sensorsObj["value"] = "ERR";

      sensorsObj["script"] = sensors[i].getSensorTopic();
      sensorsObj["cbpiid"] = sensors[i].getId();
      sensorsObj["pin"] = sensors[i].getSensPin();
      yield();
    }
  }
  else // get single sensor by id
  {
    doc["name"] = sensors[id].getSensorName();
    doc["offset1"] = sensors[id].getOffset1();
    doc["offset2"] = sensors[id].getOffset2();
    doc["sw"] = sensors[id].getSensorSwitch();
    doc["script"] = sensors[id].getSensorTopic();
    doc["cbpiid"] = sensors[id].getId();
    doc["type"] = sensors[id].getSensType();
    doc["pin"] = sensors[id].getSensPin();
  }

  char response[measureJson(doc) + 1];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void setupPT()
{
  // startSPI false := Aus, true := starte Max31865
  // sens_type 0 := DS18B20, 1 := PT100, 2 := PT1000
  // sens_pin 0 := 2-Leiter, 1 := 3-Leiter, 2 := 4-Leiter
  pins_used[SPI_MOSI] = true; // MAX31865
  pins_used[SPI_MISO] = true; // MAX31865
  pins_used[SPI_CLK] = true;  // MAX31865

  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    if (sensors[i].getSensType() > 0) // PT100x?
    {
      switch (sensors[i].getSensPin())
      {
      case 0: // 2-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(2);
        }
        break;
      case 1: // 3-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(2);
        }
        break;
      case 2: // 4-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(2);
        }
        break;
      }
    }
    pins_used[CS0] = activePT_0;
    pins_used[CS1] = activePT_1;
    pins_used[CS2] = activePT_2;
  }
}

void handleSetSenErr() // simulate sensor err
{
  simErr = server.arg(0).toInt();
#ifdef ESP32
  DEBUG_ERROR("SEN", "simulate %d", simErr);
#endif
  server.send(200);
}