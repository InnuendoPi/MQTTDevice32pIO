//    Author:	Innuendo
//
//    Kommunikation via MQTT mit CraftBeerPi v4
//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für PT100/PT1000 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Unterstützung für Nextion Touchdisplay

#if defined(ESP8266)
#pragma message "/// ESP8266 ///"
#elif defined(ESP_IDF4)
#pragma message "/// ESP32 IDF4 ///"
#elif defined(ESP_IDF5)
#pragma message "/// ESP32 IDF5 ///"
#endif

#include "MQTTDevice.h"

void configModeCallback(WiFiManager *myWiFiManager);
void setup();
void setupServer();
void loop();
void updateTools();
void updateSys();
void startHTTPUpdate();

uint32_t lastToggled = 0; // Timestamp watchdog

// Sensoren
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// IDS2 Interrupt
volatile uint8_t newError = 0; // Fehlercode IDS als Integer
uint8_t oldError = 0;

// WiFi und MQTT
#ifdef ESP32
WebServer server(80);
HTTPUpdateServer httpUpdateServer; // DateiUpdate
#elif ESP8266
WiFiEventHandler wifiConnectHandler, wifiDisconnectHandler;
ESP8266WebServer server(80);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate; // DateiUpdate
#endif
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient pubsubClient(espClient);

// Logging
#ifdef ESP32
struct InnuLogTag InnuTagLevel[LOGS_COUNT]{
    {"CFG", INNU_NONE},
    {"SEN", INNU_NONE},
    {"ACT", INNU_NONE},
    {"IND", INNU_NONE},
    {"SYS", INNU_NONE},
    {"DIS", INNU_NONE}};
#endif

struct SSESubscription subscription[SSE_MAX_CHANNELS];
uint8_t subscriptionCount = 0;

int64_t DUTYCYLCE = 5000; // Aktoren und HLT
uint16_t SENCYLCE = 1;    // Aktoren und HLT
bool senRes = false;
bool startSPI = false;
uint8_t numberOfActors = 0;  // Gesamtzahl der Aktoren
uint8_t numberOfSensors = 0; // Gesamtzahl der Sensoren
unsigned char addressesFound[NUMBEROFSENSORSMAX][8];

// GPIOS
#ifdef ESP32
bool pins_used[34] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
const int8_t pins[NUMBEROFPINS] = {26, 22, 21, 17, 16, 18, 19, 23, 5, 27, 25, 32, 12, 4, 0, 2, 33, 14, 15, 13, -100};
const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19", "-"};
#elif ESP8266
bool pins_used[17] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
const int8_t pins[NUMBEROFPINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, -100};
const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "-"};
#endif

// #ifdef ESP32
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 pt_0 = Adafruit_MAX31865(CS0, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_1 = Adafruit_MAX31865(CS1, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_2 = Adafruit_MAX31865(CS2, SPI_MOSI, SPI_MISO, SPI_CLK);
// Adafruit_MAX31865 pt_3 = Adafruit_MAX31865(CS3, SPI_MOSI, SPI_MISO, SPI_CLK);
// Adafruit_MAX31865 pt_4 = Adafruit_MAX31865(CS4, SPI_MOSI, SPI_MISO, SPI_CLK);
// Adafruit_MAX31865 pt_5 = Adafruit_MAX31865(CS5, SPI_MOSI, SPI_MISO, SPI_CLK);
// bool activePT_0 = false, activePT_1 = false, activePT_2 = false, activePT_3 = false, activePT_4 = false, activePT_5 = false;
// bool activePT_0 = false, activePT_1 = false, activePT_2 = false, activePT_3 = false, activePT_4 = false;
bool activePT_0 = false, activePT_1 = false, activePT_2 = false;
// #elif ESP8266
// Adafruit_MAX31865 pt_0 = Adafruit_MAX31865(CS0, SPI_MOSI, SPI_MISO, SPI_CLK);
// Adafruit_MAX31865 pt_1 = Adafruit_MAX31865(CS1, SPI_MOSI, SPI_MISO, SPI_CLK);
// Adafruit_MAX31865 pt_2 = Adafruit_MAX31865(CS2, SPI_MOSI, SPI_MISO, SPI_CLK);
// bool activePT_0 = false, activePT_1 = false, activePT_2 = false;
// #endif

char mqtthost[maxHostSign]; // MQTT Server
char mqttuser[maxUserSign];
char mqttpass[maxPassSign];
int mqttport;
char mqtt_clientid[maxHostSign]; // AP-Mode und Gerätename

// Zeitserver Einstellungen

char ntpServer[maxNTPSign] = NTP_ADDRESS;
char ntpZone[maxNTPSign] = NTP_ZONE;
struct tm timeinfo;
char zeit[9] = "00:00:00";

// Event handling Status Variablen
bool StopOnMQTTError = false;     // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
bool mqtt_state = true;           // Status MQTT
uint8_t wlanStatus = 0;
uint8_t simErr = 0;

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerAct;
InnuTicker TickerInd;
InnuTicker TickerMQTT;
InnuTicker TickerDisp;
InnuTicker TickerPUBSUB;
InnuTicker TickerTime;

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Systemstart
bool startMDNS = true; // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[maxHostSign] = "MQTTDevice";
bool shouldSaveConfig = false; // WiFiManager
bool alertState = false;       // WebUpdate Status
bool devBranch = false;        // Check out development branch

unsigned long lastSenAct = 0; // Timestap actors on sensor error
unsigned long lastSenInd = 0; // Timestamp induction on sensor error

int8_t sensorsStatus = 0;
int8_t actorsStatus = 0;
bool inductionStatus = false;
bool fermenterStatus = false;

bool mqttBuzzer = false; // MQTTBuzzer für CBPi4

int8_t selLang = 0; // Sprache

// Serial
SoftwareSerial softSerial;

// Display Next
bool useDisplay = false;
bool useFerm = false;
int startPage = 0;  // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
int activePage = 0; // die aktuell angezeigte Seite
int tempPage = -1;  // die aktuell angezeigte Seite
InnuNex nextion(softSerial);

int8_t PIN_BUZZER = -100;
bool current_step = false;
int stepsCounter = 0;
char currentStepName[maxStepSign];     //= "no active step";
char currentStepRemain[maxRemainSign]; //= "0:00";
char nextStepName[maxStepSign];
char nextStepRemain[maxRemainSign];
bool activeBrew = false;
char notify[maxNotifySign]; //= "Waiting for data - start brewing";
int sliderval = 0;
char uhrzeit[6] = "00:00";

struct Kettles structKettles[maxKettles];
struct Steps structSteps[maxSteps];

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file

#ifdef ESP32
// Initialisierung des Arrays max 15
Actor actors[NUMBEROFACTORSMAX] = {
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false)};
#elif ESP8266
// Initialisierung des Arrays max 10
Actor actors[NUMBEROFACTORSMAX] = {
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false)};
#endif

// Initialisierung des Arrays -> max 6 Sensoren
#ifdef ESP32
TemperatureSensor sensors[NUMBEROFSENSORSMAX] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0)};
#elif ESP8266
// Initialisierung des Arrays -> max 3 Sensoren
TemperatureSensor sensors[NUMBEROFSENSORSMAX] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0)};
#endif

induction inductionCooker = induction();

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
  Serial.println(WiFi.softAPIP());
  Serial.print("*** SYSINFO: Start configuration portal ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup()
{
  Serial.begin(DEF_SERIAL);

#ifdef ESP32
  snprintf(mqtt_clientid, maxHostSign, "ESP32-%llX", ESP.getEfuseMac());
  DEBUG_INFO("SYS", "\n*** SYSINFO: MQTTDevice32 ID: %X", mqtt_clientid);
  // WLAN Events
  WiFi.onEvent(WiFiEvent);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                                       { DEBUG_INFO("SYS", "WiFiStationDisconnected reason: %d", info.wifi_sta_disconnected.reason); },
                                       WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#elif ESP8266
  snprintf(mqtt_clientid, maxHostSign, "ESP8266-%08X", ESP.getChipId());
  Serial.printf("\n*** SYSINFO: start up MQTTDevice - device ID: %s", mqtt_clientid);
  // WLAN Events
  wifiConnectHandler = WiFi.onStationModeGotIP(EM_WIFICONNECT);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(EM_WIFIDISCONNECT);
#endif

  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter p_hint("<small>*Connect your MQTTDevice to WLAN. When connected open http://mqttdevice.local in your brower</small>");
  wifiManager.addParameter(&p_hint);
  wifiManager.autoConnect(nameMDNS);
  wifiManager.setWiFiAutoReconnect(true);
  WiFi.mode(WIFI_STA);
#ifdef ESP32
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
#elif ESP8266
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoConnect(true);
#endif

  // Lade Dateisystem
  if (LittleFS.begin())
  {
// set Logging
#ifdef ESP32
    readLog();
    DEBUG_INFO("SYS", "*** SYSINFO: setup LittleFS free heap: %d", ESP.getFreeHeap());
#endif
    // Prüfe WebUpdate
    if (LittleFS.exists(UPDATESYS))
      updateSys();
    if (LittleFS.exists(UPDATETOOLS))
      updateTools();
    // webUpdate log
    if (LittleFS.exists(LOGUPDATESYS) || LittleFS.exists(LOGUPDATETOOLS))
      EM_LOG();

    // Erstelle Ticker Objekte
    setTicker();

// ISR
#ifdef ESP32
    gpio_install_isr_service(0);
#endif

    // Starte Sensoren
    DS18B20.begin();
    DS18B20.setWaitForConversion(false); // async mode
    DS18B20.setCheckForConversion(true);
    pins_used[ONE_WIRE_BUS] = true;
    // Lade Konfiguration
    if (LittleFS.exists(CONFIG))
      loadConfig();
    else
    {
      Serial.println("*** SYSINFO: configuration file config.txt not found. Load defaults ...");
      // NTP
      setupTime();
    }
    // Starte MQTT
    EM_MQTTCONNECT();
    EM_MQTTSUBSCRIBE();
    TickerPUBSUB.start(); // Ticker PubSubClient
  }
  else
    Serial.println("*** SYSINFO: error - failed to mount LittleFS!");

  // Starte Webserver
  setupServer();

  // Starte mDNS
  EM_MDNSET();

#ifdef ESP8266
  ESP.wdtDisable();
  ESP.wdtEnable(WDT_TIMEOUT * 1000);
#endif
#ifdef ESP_IDF4                         // 2.0.17
  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch
  DEBUG_INFO("SYS", "Watchdog timeout IDF4 set to %ds", WDT_TIMEOUT);
#endif
#ifdef ESP_IDF5
  esp_task_wdt_deinit();
  // Task Watchdog configuration
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = WDT_TIMEOUT * 1000,                // Convertin ms
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Bitmask of all cores, https://github.com/espressif/esp-idf/blob/v5.2.2/examples/system/task_watchdog/main/task_watchdog_example_main.c
      .trigger_panic = true                            // Enable panic to restart ESP32
  };
  esp_err_t ESP32_ERROR = esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  DEBUG_INFO("SYS", "Watchdog timeout IDF5 set to %dms state: %s", WDT_TIMEOUT * 1000, String(esp_err_to_name(ESP32_ERROR)).c_str());
#endif
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/index.htm", handleRoot);
  server.on("/index", handleRoot);
  server.on("/index.html", handleRoot);
  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);               // GPIO Pins actors
  server.on("/reqIndu", handleRequestIndu);           // Induction für WebConfig
  server.on("/setSensor", handleSetSensor);           // Sensor ändern
  server.on("/setSenErr", handleSetSenErr);           // Sensor ändern
  server.on("/setActor", handleSetActor);             // Aktor ändern
  server.on("/setIndu", handleSetIndu);               // Indu ändern
  server.on("/delSensor", handleDelSensor);           // Sensor löschen
  server.on("/delActor", handleDelActor);             // Aktor löschen
  server.on("/reboot", EM_REBOOT);                    // reboots the whole Device
  server.on("/reqMisc", handleRequestMisc);           // Misc Infos für WebConfig
  server.on("/reqMisc2", handleRequestMisc2);         // Misc Infos für WebConfig
  server.on("/reqMiscAlert", handleRequestMiscAlert); // Misc Infos für WebConfig
  server.on("/reqFirm", handleRequestFirm);           // Firmware version
  server.on("/setMisc", handleSetMisc);               // Misc ändern
  server.on("/setMiscLang", handleSetMiscLang);       // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate);     // Firmware WebUpdate
  server.on("/channel", handleChannel);               // Server Sent Events will be handled from this URI
  server.on("/startSSE", startSSE);                   // Server Sent Events will be handled from this URI
  server.on("/checkAliveSSE", checkAliveSSE);         // Server Sent Events check IP on channel
  server.on("/language", handleGetLanguage);
  server.on("/title", handleGetTitle);
  server.on("/reqSys", handleReqSys);
  // FSBrowser initialisieren
  server.on("/edit", HTTP_GET, handleGetEdit);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on(
      "/edit", HTTP_POST, []()
      { server.send(200); },
      handleFileUpload);
  server.on(
      "/restore", HTTP_POST, []()
      { server.send(200); },
      handleRestore);

  server.on("/rest/events/0", handleAll);
  server.on("/rest/events/1", handleAll);
  server.on("/rest/events/2", handleAll);
  server.on("/rest/events/3", handleAll);
  server.on("/rest/events/4", handleAll);
  server.on("/rest/events/5", handleAll);

  // server.serveStatic("/mqttdevice.min.css", LittleFS, "/mqttdevice.min.css", "public, max-age=86400");
  // server.serveStatic("/mqttdevice.min.js", LittleFS, "/mqttdevice.min.js", "public, max-age=86400");
  // server.serveStatic("/mqttfont.ttf", LittleFS, "/mqttfont.ttf", "public, max-age=86400");
  server.serveStatic("/lang.js", LittleFS, "/lang.js", "public, max-age=86400");
  server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico", "public, max-age=86400");
  server.serveStatic("/de.json", LittleFS, "/de.json", "no-store, must-revalidate");
  server.serveStatic("/en.json", LittleFS, "/en.json", "no-store, must-revalidate");
  server.serveStatic("/config.txt", LittleFS, "/config.txt", "no-cache, no-store, must-revalidate");
  server.serveStatic("/log_cfg.json", LittleFS, "/log_cfg.json", "no-cache, no-store, must-revalidate");

  server.onNotFound(handleAll);
#ifdef ESP32
  httpUpdateServer.setup(&server); // DateiUpdate
#elif ESP8266
  httpUpdate.setup(&server);
#endif
  server.begin();
}

void loop()
{
  server.handleClient(); // Webserver handle
  if (millis() > (lastToggled + WDT_TIMEOUT / 2))
  {
    lastToggled = millis();
#ifdef ESP32
    esp_task_wdt_reset();
    delay(1);
#endif
#ifdef ESP8266
    ESP.wdtFeed();
#endif
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    TickerPUBSUB.update(); // Ticker PubSubClient
#ifdef ESP8266
    if (startMDNS) // ESP8266 mDNS handle
      mdns.update();
#endif
  }

  TickerTime.update();

  if (numberOfSensors > 0) // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0) // Ticker Aktoren
    TickerAct.update();
  if (inductionStatus) // Ticker Induktion
    TickerInd.update();
  if (useDisplay) // Ticker Display
  {
    TickerDisp.update();
    nextion.checkNex();
  }
}
