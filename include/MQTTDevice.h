#ifndef MQTTDEVICE_H
#define MQTTDEVICE_H

#include <Arduino.h>
#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <Adafruit_MAX31865.h>
#ifdef ESP32
#include <WiFi.h>             // Generelle WiFi Funktionalität
#include <WebServer.h>        // Unterstützung Webserver
#include <HTTPUpdateServer.h> // DateiUpdate
#include <ESPmDNS.h>          // mDNS
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <esp_task_wdt.h>
#elif ESP8266
#include <ESP8266WiFi.h>      // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>        // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h> // DateiUpdate
#include <ESP8266mDNS.h>      // mDNS
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#endif
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>          // WiFi
#include <WiFiManager.h> // WiFiManager zur Einrichtung
#include <DNSServer.h>   // Benötigt für WiFiManager
#include <LittleFS.h>    // Dateisystem
#include <FS.h>          // Files
#include <ArduinoJson.h> // Lesen und schreiben von JSON Dateien
#include <Ticker.h>
#include <time.h>
#include <PubSubClient.h>   // MQTT Kommunikation
#include <SoftwareSerial.h> // Serieller Port für Display
#include "InnuTicker.h"     // Bibliothek für Hintergrund Aufgaben (Tasks)
#include "InnuNextion.h"
// #include "index_htm.h"

// Version
#ifdef ESP_IDF5
#define Version "4.70 pIO"
#else
#define Version "4.70"
#endif

// Watchdog
#define WDT_TIMEOUT 60 // Watchdog timeout in sek

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750
#define RESOLUTION_HIGH 12 // steps 9bit: 0.5°C 10bit: 0.25°C 11bit: 0.125°C 12bit: 0.0625°C
#define RESOLUTION 11
#define TEMP_OFFSET1 40
#define TEMP_OFFSET2 78
#define maxHostSign 31
#define maxUserSign 16
#define maxPassSign 16
#define maxNTPSign 31
#define NTP_ADDRESS "europe.pool.ntp.org" // NTP Server
#define NTP_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#define SCHALTJAHR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

#define SSE_MAX_CHANNELS 6 // 6 SSE clients subscription erlaubt
#define PORT 80

// extern struct InnuLogTag InnuTagLevel[LOGS_COUNT];
// #ifdef ESP8266
// extern const char *IRAM_ATTR pathToFileName(const char *path);
// #endif

// SSE broadcasts
#define SENSORJSON 0
#define ACTORJSON 1
#define INDJSON 2
#define MISCJSON 3


// Update Intervalle für Ticker Objekte
#define SEN_UPDATE 1000  //  sensors update
#define ACT_UPDATE 1000  //  actors update
#define IND_UPDATE 1000  //  induction update
#define DISP_UPDATE 1000 //  display update
#define TIME_UPDATE 30000
// Event handling Zeitintervall für Reconnects WLAN und MQTT
#define tickerMQTT 10000 // für Ticker Objekt MQTT in ms
#define tickerPUSUB 50   // Ticker PubSubClient
#define DEF_DELAY_IND 120000 // Standard Nachlaufzeit nach dem Ausschalten Induktionskochfeld

// Sensoren
#define RREF1000 4300.0
#define RNOMINAL1000 1000.0
#define RREF100 430.0
#define RNOMINAL100 100.0

// Event für Sensoren
#define EM_OK 0    // Normal mode
#define EM_CRCER 1 // Sensor CRC failed
#define EM_DEVER 2 // Sensor device error
#define EM_UNPL 3  // Sensor unplugged
#define EM_SENER 4 // Sensor all errors

// System Dateien
#define UPDATESYS "/updateSys.txt"
#define LOGUPDATESYS "/updateSys.log"
#define UPDATETOOLS "/updateTools.txt"
#define LOGUPDATETOOLS "/updateTools.log"
#define UPDATELOG "/webUpdateLog.txt"
#define DEVBRANCH "/dev.txt"
#define CONFIG "/config.txt"
#define LOG_CFG "/log_cfg.json"

#ifdef ESP32
#define Aus -100
#define D0 26
#define D1 22
#define D2 21
#define D3 17
#define D4 16
#define D5 18
#define D6 19
#define D7 23
#define D8 5
#define D9 27
#define D10 25
#define D11 32
#define D12 12
#define D13 4
#define D14 0
#define D15 2
#define D16 33
#define D17 14
#define D18 15
#define D19 13
// #define D20 10
#endif

// GPIOS
#ifdef ESP32
#define NUMBEROFPINS 21
extern bool pins_used[34];                   // = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, 0};
extern const int8_t pins[NUMBEROFPINS];      // = {26, 22, 21, 17, 16, 18, 19, 23, 5, 27, 25, 32, 12, 4, 0, 2, 33, 14, 15, 13, -100};
extern const String pin_names[NUMBEROFPINS]; // = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19", "-"};
#elif ESP8266
#define NUMBEROFPINS 10
extern bool pins_used[17]; // GPIO
extern const int8_t pins[NUMBEROFPINS]; // = {D0, D1, D2, D3, D4, D5, D6, D7, D8, -100};
extern const String pin_names[NUMBEROFPINS]; // = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "-"};
#endif

// Variablen
#ifdef ESP32
#define NUMBEROFSENSORSMAX 6 // Maximale Anzahl an Sensoren
#define NUMBEROFACTORSMAX 15 // Maximale Anzahl an Aktoren
#elif ESP8266
#define NUMBEROFSENSORSMAX 3 // Maximale Anzahl an Sensoren
#define NUMBEROFACTORSMAX 10 // Maximale Anzahl an Aktoren
#endif

#define ONE_WIRE_BUS D3
extern int8_t PIN_BUZZER;

extern uint32_t lastToggled;

#ifdef ESP32
extern WebServer server;
extern HTTPUpdateServer httpUpdateServer; // DateiUpdate
#elif ESP8266
extern ESP8266WebServer server;
extern MDNSResponder mdns;
extern ESP8266HTTPUpdateServer httpUpdate; // DateiUpdate
extern WiFiEventHandler wifiConnectHandler, wifiDisconnectHandler;
extern uint8_t wlanStatus;
#endif

// Event handling Status Variablen
extern bool StopOnMQTTError;     // Event handling für MQTT Fehler
extern unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
extern bool mqtt_state;           // Status MQTT
extern uint8_t simErr;

// OneWire
extern OneWire oneWire;
extern DallasTemperature DS18B20;
extern bool startSPI;
extern volatile uint8_t newError; // Fehlercode IDS als Integer
extern uint8_t oldError;
extern uint16_t SENCYLCE; // Aktoren und HLT
extern bool senRes;
extern bool startSPI;
extern uint8_t numberOfSensors; // Gesamtzahl der Sensoren
extern unsigned char addressesFound[NUMBEROFSENSORSMAX][8];

extern int64_t DUTYCYLCE; // Aktoren und HLT
extern uint8_t numberOfActors;  // Gesamtzahl der Aktoren

// WiFi
extern WiFiManager wifiManager;
extern WiFiClient espClient;
extern PubSubClient pubsubClient;

extern char mqtthost[maxHostSign]; // MQTT Server
extern char mqttuser[maxUserSign];
extern char mqttpass[maxPassSign];
extern int mqttport;
extern char mqtt_clientid[maxHostSign]; // AP-Mode und Gerätename

// Zeitserver Einstellungen

extern char ntpServer[maxNTPSign];
extern char ntpZone[maxNTPSign];
extern struct tm timeinfo;
extern char zeit[9];

// Logging
#ifdef ESP32
#define DEBUG_ESP_PORT Serial
#define LOG_COLOR_ERROR "\033[0;31m"   // Red
#define LOG_COLOR_INFO "\033[0;32m"    // green
#define LOG_COLOR_WARN "\033[0;33m"    // blue
#define LOG_COLOR_VERBOSE "\033[0;36m" // cyan
#define LOG_COLOR_RESET "\033[0m"      // Text reset

#define INNU_NONE 0
#define INNU_ERROR 1
#define INNU_INFO 2
#define INNU_VERBOSE 3

#define LOGS_COUNT 6

struct InnuLogTag
{
    String tagName;
    int level;
};

extern struct InnuLogTag InnuTagLevel[LOGS_COUNT];
// #ifdef ESP8266
// extern const char *IRAM_ATTR pathToFileName(const char *path);
// #endif

#define DEBUG_ERROR(TAG, ...)                                                                                               \
    if (getTagLevel(TAG) >= INNU_ERROR)                                                                                     \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_ERROR);                                                                             \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][E][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }

#define DEBUG_INFO(TAG, ...)                                                                                                \
    if (getTagLevel(TAG) >= INNU_INFO)                                                                                      \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_INFO);                                                                              \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][I][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }

#define DEBUG_VERBOSE(TAG, ...)                                                                                             \
    if (getTagLevel(TAG) >= INNU_VERBOSE)                                                                                   \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_VERBOSE);                                                                           \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][V][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }
#endif

// 6 SSE clients subscription
struct SSESubscription
{
  IPAddress clientIP = INADDR_NONE;
  WiFiClient client;
  Ticker keepAliveTimer;
  bool check = false;
};
extern struct SSESubscription subscription[SSE_MAX_CHANNELS];
extern uint8_t subscriptionCount;

// Ticker Objekte
extern InnuTicker TickerSen;
extern InnuTicker TickerAct;
extern InnuTicker TickerInd;
extern InnuTicker TickerMQTT;
extern InnuTicker TickerDisp;
extern InnuTicker TickerPUBSUB;
extern InnuTicker TickerTime;

// Event handling Standard Verzögerungen
extern unsigned long wait_on_error_mqtt;             // How long should device wait between tries to reconnect WLAN      - approx in ms
extern unsigned long wait_on_Sensor_error_actor;     // How long should actors wait between tries to reconnect sensor    - approx in ms
extern unsigned long wait_on_Sensor_error_induction; // How long should induction wait between tries to reconnect sensor - approx in ms

// Systemstart
extern bool startMDNS;          // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
extern char nameMDNS[maxHostSign];
extern bool shouldSaveConfig; // WiFiManager
extern bool alertState;       // WebUpdate Status
extern bool devBranch;        // Check out development branch

extern unsigned long lastSenAct;   // Timestap actors on sensor error
extern unsigned long lastSenInd;   // Timestamp induction on sensor error

extern int8_t sensorsStatus;
extern int8_t actorsStatus;
extern bool inductionStatus;
extern bool fermenterStatus;

extern bool mqttBuzzer;  // MQTTBuzzer für CBPi4

extern int8_t selLang;       // Sprache
// FSBrowser
extern File fsUploadFile; // a File object to temporarily store the received file

#ifdef ESP32
// PT100x
#define SPI_MOSI D11
#define SPI_MISO D10
#define SPI_CLK D9
#define CS0 D13
#define CS1 D16
#define CS2 D17
#define CS3 D18
#define CS4 D19
#define CS5 D8
extern Adafruit_MAX31865 pt_0;
extern Adafruit_MAX31865 pt_1;
extern Adafruit_MAX31865 pt_2;
extern Adafruit_MAX31865 pt_3;
extern Adafruit_MAX31865 pt_4;
extern Adafruit_MAX31865 pt_5;
extern bool activePT_0, activePT_1, activePT_2, activePT_3, activePT_4, activePT_5;
#elif ESP8266
#define SPI_MOSI D0
#define SPI_MISO D1
#define SPI_CLK D2
#define CS0 D4
#define CS1 D5
#define CS2 D6
extern Adafruit_MAX31865 pt_0;
extern Adafruit_MAX31865 pt_1;
extern Adafruit_MAX31865 pt_2;
extern bool activePT_0, activePT_1, activePT_2;
#endif

// Display
extern SoftwareSerial softSerial;

#define NUMBEROFPAGES 3
#define DEF_SERIAL 115200
#define DEF_NEXTION 9600
extern bool useDisplay;
extern bool useFerm;
extern int startPage;  // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
extern int activePage; // die aktuell angezeigte Seite
extern int tempPage;  // die aktuell angezeigte Seite
extern InnuNex nextion; 

// KettlePage
#define uhrzeit_text "clock.txt"
#define currentStepName_text "currentStep.txt"
#define currentStepRemain_text "currentRemain.txt"
#define nextStepName_text "nextStep.txt"
#define nextStepRemain_text "nextTimer.txt"
#define kettleName1_text "Kettle1.txt"
#define kettleIst1_text "temp1.txt"
#define kettleSoll1_text "target1.txt"
#define kettleName2_text "Kettle2.txt"
#define kettleIst2_text "temp2.txt"
#define kettleSoll2_text "target2.txt"
#define kettleName3_text "Kettle3.txt"
#define kettleIst3_text "temp3.txt"
#define kettleSoll3_text "target3.txt"
#define kettleName4_text "Kettle4.txt"
#define kettleIst4_text "temp4.txt"
#define kettleSoll4_text "target4.txt"
#define progress "j0.txt"
#define mqttDevice "IP.txt"
#define notification "notification1.txt"
// BrewPage
#define p1uhrzeit_text "p1clock.txt"
#define p1current_text "p1current.txt"
#define p1remain_text "p1remain.txt"
#define p1temp_text "p1temp.txt"
#define p1target_text "p1target.txt"
#define p1progress "j0.txt"
#define p1mqttDevice "IP.txt"
#define p1notification "p1notification.txt"
// InductionPage
#define powerButton "buttonOnOff.val"
#define p2uhrzeit_text "p2clock.txt"
#define p2slider "sliderPower.val"
#define p2temp_text "textTemp.txt"
#define p2gauge "gauge.val"

// CraftbeerPi4 definitions
#define maxKettles 4
#define maxKettleSign 15
#define maxIdSign 23
#define maxSensorSign 23
#define maxStepSign 30
#define maxRemainSign 10
#define maxNotifySign 52
#define maxTempSign 10

#define maxTopicSigns 42
#define cbpi4steps_topic "cbpi/stepupdate/+"    // SmqhKAMS6Z6ExTj9wa7y68
#define cbpi4kettle_topic "cbpi/kettleupdate/+" // BGAEZHXmHUfT44SLNV2xbF
#define cbpi4sensor_topic "cbpi/sensordata/"    // mKdeC6LjHZmz9Sa2mVf5SV
#define cbpi4actor_topic "cbpi/actorupdate/+"
#define cbpi4notification_topic "cbpi/notification"
#define cbpi4fermenter_topic "cbpi/fermenterupdate/+"
// #define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/YdceEBous3wSRUQsTx9ZLq/+"
// #define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/+/+"
#define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/+"

// Alarm codes
#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_INFO 3
#define ALARM_SUCCESS 4
#define ALARM_WARNING 5
#define ALARM_ERROR 6

// Select
#define OPTIONSTART "<option>"
#define OPTIONEND "</option>"
#define OPTIONDISABLED "</option><option disabled>──────────</option>"
#define TRENNLINIE "-----------------------"

#include "InnuLog.h"
#include "SYSTEM.h"
#include "AKTOREN.h"
#include "CONFIGFILE.h"
#include "DISPLAY.h"
#include "FSBrowser.h"
#include "INDUKTION.h"
#include "SENSOREN.h"
#include "SSE.h"
#include "TICKERCALLBACK.h"
#include "WEB.h"
#include "WEBUPDATE.h"

extern bool current_step;
struct Kettles
{
    char id[maxIdSign];
    char name[maxKettleSign];
    char current_temp[maxTempSign];
    char target_temp[maxTempSign];
    char sensor[maxSensorSign];
};
extern struct Kettles structKettles[maxKettles];

// extern struct SSESubscription subscription[SSE_MAX_CHANNELS];
// struct SSESubscription subscription[SSE_MAX_CHANNELS];

#define maxSteps 15
struct Steps
{
    char id[maxIdSign];
    char name[maxStepSign];
    char timer[maxRemainSign];
};
extern struct Steps structSteps[maxSteps];
extern int stepsCounter;

extern char currentStepName[maxStepSign];     //= "no active step";
extern char currentStepRemain[maxRemainSign]; //= "0:00";
extern char nextStepName[maxStepSign];
extern char nextStepRemain[maxRemainSign];
extern bool activeBrew;

extern char notify[maxNotifySign]; //= "Waiting for data - start brewing";
extern int sliderval;
extern char uhrzeit[6];

extern Actor actors[];
extern TemperatureSensor sensors[];
extern induction inductionCooker;

#endif // #ifndef MQTTDEVICE_H