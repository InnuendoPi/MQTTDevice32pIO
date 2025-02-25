#ifndef INDUKTION_H
#define INDUKTION_H

#include <Arduino.h>

class induction
{
private:
    int8_t PIN_WHITE = D7;     // D7 Relay white
    int8_t PIN_YELLOW = D6;    // D6 Command channel yellow AUSGABE AN PLATTE
    int8_t PIN_INTERRUPT = D5; // D5 Back channel blue EINGABE VON PLATTE
    uint8_t power = 0;
    uint8_t newPower = 0;
    uint8_t oldPower = 0;
    int8_t CMD_CUR = 0;           // Aktueller Befehl
    boolean isRelayon = false;    // Systemstatus: ist das Relais in der Platte an?
    boolean oldisRelayon = false; // Systemstatus: ist das Relais in der Platte an?
    boolean isInduon = false;     // Systemstatus: ist Power > 0?
    boolean oldisInduon = false;
    boolean isPower = false;
    String mqtttopic = "";
    boolean isEnabled = false;
    uint8_t powerLevelOnError = 100;   // 100% schaltet das Event handling für Induktion aus
    uint8_t powerLevelBeforeError = 0; // in error event save last power state
    bool induction_state = true;       // Error state induction

    unsigned long timeTurnedoff;
    unsigned long lastInterrupt;
    unsigned long lastCommand;
    bool inputStarted = false;
    uint8_t inputCurrent = 0;
    unsigned char inputBuffer[33];
    long powerSampletime = 20000;
    unsigned long powerLast;
    long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
    long powerLow = 0;
    // Induktion Signallaufzeiten
    const int16_t SIGNAL_HIGH = 5120;
    const int16_t SIGNAL_HIGH_TOL = 1500;
    const int16_t SIGNAL_LOW = 1280;
    const int16_t SIGNAL_LOW_TOL = 500;
    const int16_t SIGNAL_START = 25;
    const int16_t SIGNAL_START_TOL = 10;
    const int16_t SIGNAL_WAIT = 10;
    const int16_t SIGNAL_WAIT_TOL = 5;

    /*  Binäre Signale für Induktionsplatte */
    int16_t CMD[6][33] = {
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
    uint8_t PWR_STEPS[6] = {0, 20, 40, 60, 80, 100};

public:
    double ids2Input, ids2Output, ids2Setpoint;
    induction();

    void change(const int8_t &pinwhite, const int8_t &pinyellow, const int8_t &pinblue, const String &topic, const bool &is_enabled, const uint8_t &powerLevel);
    void mqtt_subscribe();
    void mqtt_unsubscribe();
    void handlemqtt(unsigned char *payload, unsigned int length);
    void setupCommands();
    bool updateRelay();
    void Update();
    void inductionNewPower(const int16_t &val);
    void updatePower();
    void sendCommand(int16_t command[33]);
    void readInput();
    void indERR();
    int8_t getPinWhite();
    int8_t getPinYellow();
    int8_t getPinInterrupt();
    void setPinWhite(const int8_t &val);
    void setPinYellow(const int8_t &val);
    void setPinInterrupt(const int8_t &val);
    String getTopic();
    uint8_t getPower();
    uint8_t getOldPower();
    void setOldPower();
    int8_t getCMD_CUR();
    uint8_t getNewPower();
    void setNewPower(const int16_t &val);
    bool getisRelayon();
    bool getoldisRelayon();
    void setoldisRelayon();
    bool getisInduon();
    void setisInduon(const bool &val);
    bool getoldisInduon();
    void setoldisInduon();
    bool getisPower();
    bool getIsEnabled();
    void setIsEnabled(const bool &val);
    bool getInductionState();
    void setInductionState(const bool &val);
    uint8_t getPowerLevelOnError();
    uint8_t getPowerLevelBeforeError();
};

#ifdef ESP32
void ARDUINO_ISR_ATTR readInputWrap();
#elif ESP8266
ICACHE_RAM_ATTR void readInputWrap();
#endif
void handleInduction();
void handleRequestInduction();
void handleRequestIndu();
void handleSetIndu();
void checkIDSstate();

#endif