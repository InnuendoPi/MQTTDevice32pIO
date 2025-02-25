// WebUpdate Board configuration ESP8266
// "configuration": "xtal=80,vt=flash,exception=disabled,stacksmash=disabled,ssl=basic,mmu=4816H,non32xfer=fast,eesz=4M2M,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600",

#include "MQTTDevice.h"

bool upTools(const String &url, const String &fname, WiFiClientSecure &clientup)
{
    HTTPClient https;
    char line[120];
    millis2wait(100);
    if (https.begin(clientup, url + fname))
    {
        int16_t httpCode = https.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                uint8_t buff[128] = {0};
                WiFiClient *stream = https.getStreamPtr(); // get tcp stream
                bool check = LittleFS.remove("/" + fname);
                fsUploadFile = LittleFS.open("/" + fname, "w");
                int32_t len = https.getSize();
                int32_t startlen = len;
                while (https.connected() && (len > 0 || len == -1))
                {
                    size_t size = stream->available(); // get available data size
                    if (size > 0)
                    {
                        int32_t c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // lese maximal 128 byte in buff
                        fsUploadFile.write(buff, c);                                                        // schreibe buff in Datei
                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                fsUploadFile.close();
                fsUploadFile = LittleFS.open("/" + fname, "r");
                if (fsUploadFile.size() == startlen)
                {
                    snprintf(line, sizeof(line), "Framwork/Tools update %s getSize: %d fileSize: %d", fname.c_str(), startlen, fsUploadFile.size());
                    debugLog(UPDATELOG, line);
#ifdef ESP32
                    DEBUG_INFO("SYS", "%s", line);
#endif
                }
                else
                {
                    snprintf(line, sizeof(line), "Framwork/Tools update error %s getSize: %d fileSize: %d", fname.c_str(), startlen, fsUploadFile.size());
                    debugLog(UPDATELOG, line);
#ifdef ESP32
                    DEBUG_ERROR("SYS", "%s", line);
#endif
                }
                fsUploadFile.close();
                https.end();
                return true;
            }
            else
                return false;
        }
        else
        {
            snprintf(line, sizeof(line), "Framwork/Tools2 update error %s %s", fname.c_str(), https.errorToString(httpCode).c_str());
            debugLog(UPDATELOG, line);
#ifdef ESP32
            DEBUG_ERROR("SYS", "%s", line);
#endif
            https.end();
            return false;
        }
    }
    else
    {
        snprintf(line, sizeof(line), "Framwork/Tools3 update Fehler https start %s", fname.c_str());
        debugLog(UPDATELOG, line);
#ifdef ESP32
        DEBUG_ERROR("SYS", "%s", line);
#endif
        return false;
    }
}

void upFirm()
{
    char line[120];
    WiFiClientSecure clientup;
    clientup.setInsecure();

#ifdef ESP_IDF5
    // httpUpdate.followRedirects(true);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onError(update_error);
    t_httpUpdate_return ret;

    if (LittleFS.exists(DEVBRANCH))
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/development/build/ESP32-IDF5/firmware.bin");
    else
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/main/build/ESP32-IDF5/firmware.bin");
#endif
#ifdef ESP_IDF4
    // httpUpdate.followRedirects(true);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onError(update_error);
    t_httpUpdate_return ret;

    if (LittleFS.exists(DEVBRANCH))
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/development/build/ESP32-IDF4/firmware.bin");
    else
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/main/build/ESP32-IDF4/firmware.bin");
#endif
#ifdef ESP8266
    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onError(update_error);
    t_httpUpdate_return ret;

    if (LittleFS.exists(DEVBRANCH))
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/development/build/ESP8266/firmware.bin");
    else
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/main/build/ESP8266/firmware.bin");
#endif

    return;
}

void updateTools()
{
#ifdef ESP32
    setTagLevel("SYS", INNU_INFO);
#endif
    configTzTime(ntpZone, ntpServer);
    getLocalTime(&timeinfo);

    WiFiClientSecure clientup;
    fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "r");
    int8_t anzahlVersuche = 0;
    if (fsUploadFile)
    {
        char anzahlV = char(fsUploadFile.read()) - '0';
        anzahlVersuche = (int8_t)anzahlV;
    }
    fsUploadFile.close();
    if (anzahlVersuche > 3)
    {
        LittleFS.remove(UPDATETOOLS);
#ifdef ESP32
        DEBUG_ERROR("SYS", "ERROR update tools - %d", anzahlVersuche);
#endif
        return;
    }
    anzahlVersuche++;
    fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "w");
    int32_t bytesWritten = fsUploadFile.print((anzahlVersuche));
    fsUploadFile.close();
#ifdef ESP32
    DEBUG_INFO("SYS", "ESP IP Adresse: %s Zeit: %s WLAN Signal: %d", WiFi.localIP().toString().c_str(), zeit, WiFi.RSSI());
#endif
    char line[120];
    snprintf(line, sizeof(line), "Update Framework/Tools #%d gestartet - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
    debugLog(UPDATELOG, line);
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", line);
#endif
    if (anzahlVersuche == 1)
    {
        snprintf(line, sizeof(line), "Firmware Version: %s", Version);
        debugLog(UPDATELOG, line);
#ifdef ESP32
        DEBUG_INFO("SYS", "%s", line);
#endif
    }
    clientup.setInsecure();
    bool test;
    if (LittleFS.exists(DEVBRANCH))
    {
        String upPath = "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/development/data/";
        test = upTools(upPath, "brautomat.min.css.gz", clientup);
        test = upTools(upPath, "brautomat.min.js.gz", clientup);
        test = upTools(upPath, "brautomat.ttf.gz", clientup);
        test = upTools(upPath, "favicon.ico", clientup);
        test = upTools(upPath, "lang.js", clientup);
        test = upTools(upPath, "language/de.json", clientup);
        test = upTools(upPath, "language/en.json", clientup);
    }
    else
    {
        String upPath = "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32pIO/main/data/";
        test = upTools(upPath, "brautomat.min.css.gz", clientup);
        test = upTools(upPath, "brautomat.min.js.gz", clientup);
        test = upTools(upPath, "brautomat.ttf.gz", clientup);
        test = upTools(upPath, "favicon.ico", clientup);
        test = upTools(upPath, "lang.js", clientup);
        test = upTools(upPath, "language/de.json", clientup);
        test = upTools(upPath, "language/en.json", clientup);
    }

    LittleFS.remove(UPDATETOOLS);
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void updateSys()
{
#ifdef ESP32
    setTagLevel("SYS", INNU_INFO);
#endif
    configTzTime(ntpZone, ntpServer);
    getLocalTime(&timeinfo);

    fsUploadFile = LittleFS.open(LOGUPDATESYS, "r");
    int8_t anzahlVersuche = 0;
    if (fsUploadFile)
    {
        char anzahlV = char(fsUploadFile.read()) - '0';
        anzahlVersuche = (int8_t)anzahlV;
    }
    fsUploadFile.close();
    if (anzahlVersuche > 3)
    {
        LittleFS.remove(UPDATESYS);
        return;
    }
    fsUploadFile = LittleFS.open(LOGUPDATESYS, "w");
    anzahlVersuche++;
    int32_t bytesWritten = fsUploadFile.print(anzahlVersuche);
    fsUploadFile.close();

    char line[120];
    snprintf(line, sizeof(line), "WebUpdate firmware #%d gestartet - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
    debugLog(UPDATELOG, line);
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", line);
#endif
    snprintf(line, sizeof(line), "Firmware Version: %s", Version);
    debugLog(UPDATELOG, line);
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", line);
#endif
    upFirm();
}

void startHTTPUpdate()
{
    server.send(200);
    fsUploadFile = LittleFS.open(UPDATESYS, "w");
    if (!fsUploadFile)
    {
#ifdef ESP32
        DEBUG_ERROR("SYS", "%s", "Error WebUpdate firmware create file (LittleFS)");
#endif
        return;
    }
    else
    {
#ifdef ESP32
        DEBUG_INFO("SYS", "%s", "WebUpdate firmware create file (LittleFS)");
#endif
        int32_t bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
    char line[120];
    bool check = LittleFS.remove(UPDATELOG);
    debugLog(UPDATELOG, "WebUpdate gestartet");
    snprintf(line, sizeof(line), "Aktuelle Version: %s", Version);
    debugLog(UPDATELOG, line);
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", line);
#endif
    if (devBranch)
    {
        fsUploadFile = LittleFS.open(DEVBRANCH, "w");
        if (!fsUploadFile)
        {
#ifdef ESP32
            DEBUG_ERROR("SYS", "%s", "Error WebUpdate firmware dev create file (LittleFS)");
#endif
            return;
        }
        else
        {
#ifdef ESP32
            DEBUG_INFO("SYS", "%s", "WebUpdate firmware dev create file (LittleFS)");
#endif
            int32_t bytesWritten = fsUploadFile.print("0");
            debugLog(UPDATELOG, "WebUpdate development branch ausgewählt");
            fsUploadFile.close();
        }
    }
    else
    {
        if (LittleFS.exists(DEVBRANCH))
            bool check = LittleFS.remove(DEVBRANCH);
    }
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", "WebUpdate Firmware reboot");
#endif
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_finished()
{
    debugLog(UPDATELOG, "Firmware erfolgreich aktualisiert");
#ifdef ESP32
    DEBUG_INFO("SYS", "%s", "Firmware aktualisiert");
#endif
    fsUploadFile = LittleFS.open(UPDATETOOLS, "w");
    if (fsUploadFile)
    {
        int32_t bytesWritten = fsUploadFile.print(0);
        fsUploadFile.close();
    }
    LittleFS.remove(UPDATESYS);
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_error(int16_t err)
{
    char line[120];
#ifdef ESP32
    snprintf(line, sizeof(line), "Firmware Update Fehler %d: %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#elif ESP8266
    snprintf(line, sizeof(line), "Firmware update error %d: %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif
    debugLog(UPDATELOG, line);
#ifdef ESP32
    DEBUG_ERROR("SYS", "%s", line);
#endif
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}
