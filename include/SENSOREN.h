#ifndef SENSOREN_H
#define SENSOREN_H

#include <Arduino.h>

// Sensoren
class TemperatureSensor
{
private:
  int8_t sens_err = 0;
  bool sens_sw = false;          // Events aktivieren
  bool sens_state = true;        // Fehlerstatus ensor
  bool old_state = true;         // Fehlerstatus ensor
  bool sens_isConnected;         // ist der Sensor verbunden
  float sens_offset1 = 0.0;      // Offset - Temp kalibrieren
  float sens_offset2 = 0.0;      // Offset - Temp kalibrieren
  float sens_value = 0.0;        // Aktueller Wert
  float old_value = 0.0;         // Aktueller Wert
  String sens_name;              // Name für Anzeige auf Website
  char sens_mqtttopic[50];       // Für MQTT Kommunikation
  uint8_t sens_address[8]; // 1-Wire Adresse
  String sens_id;
  char buf[8];
  uint8_t sens_type = 0; // 0 := DS18B20, 1 := PT100, 2 := PT1000
  uint8_t sens_pin = 0;  // 0 := 2-cable, 1 := 3-cable, 2 := 4-cable
  int8_t sens_ptid = -1; // PT-Sensors ID

public:
    TemperatureSensor(String new_address, String new_mqtttopic, String new_name, String new_id, float new_offset1, float new_offset2, bool new_sw, uint8_t new_type, uint8_t new_pin);

    void Update();

    void change(const String &new_address, const String &new_mqtttopic, const String &new_name, const String &new_id, float new_offset1, float new_offset2, const bool &new_sw, const uint8_t &new_type, const uint8_t &new_pin);
    void publishmqtt();
    int8_t getErr();
    bool getSensorSwitch();
    bool getSensorState();
    float getOffset1();
    float getOffset2();
    float getValue();
    float oldValue();
    void setOldValue();
    String getSensorName();
    String getSensorTopic();
    String getId();
    char *getValueString();
    float getTotalValueFloat();
    char *getTotalValueString();
    float calcOffset();
    String getSens_adress_string();
    uint8_t getSensType();
    void setSensType(const uint8_t &val);
    uint8_t getSensPin();
    void setSensPTid(const int8_t &val);
};

void handleSensors(bool checkSen);                          
uint8_t searchSensors();                                    
String SensorAddressToString(uint8_t addr[8]);
void handleSetSensor();                                     
void handleDelSensor();                                     
void handleRequestSensorAddresses();                        
void handleRequestSensors();                                
void setupPT();  
void handleSetSenErr();
#endif