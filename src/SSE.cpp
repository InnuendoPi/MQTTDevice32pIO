#include "MQTTDevice.h"

void startSSE()
{
    server.send(200);
    handleSensors(true);
    TickerSen.setLastTime(millis());
    handleActors(true);
    TickerAct.setLastTime(millis());
    inductionSSE(true);
    TickerInd.setLastTime(millis());
    miscSSE();
}

void inductionSSE(bool val)
{
    // val true: init
    // val false: only updates

    JsonDocument ssedoc;
    ssedoc["enabled"] = (int)inductionCooker.getIsEnabled();
    ssedoc["power"] = 0;
    if (inductionCooker.getIsEnabled())
    {
        ssedoc["relayOn"] = inductionCooker.getisRelayon();
        ssedoc["power"] = inductionCooker.getPower();
        ssedoc["state"] = inductionCooker.getInductionState();
        if (inductionCooker.getisPower())
        {
            ssedoc["powerLevel"] = inductionCooker.getCMD_CUR();
        }
        else
        {
            ssedoc["powerLevel"] = max(0, inductionCooker.getCMD_CUR() - 1);
        }
    }
    else
        val = false;

    ssedoc["topic"] = inductionCooker.getTopic();
    ssedoc["pl"] = inductionCooker.getPowerLevelOnError();

    if (!val)
    {
        if (inductionCooker.getOldPower() != inductionCooker.getPower())
        {
            inductionCooker.setOldPower();
            val = true;
        }
        else if (inductionCooker.getoldisRelayon() != inductionCooker.getisRelayon())
        {
            inductionCooker.setoldisRelayon();
            val = true;
        }
        else if (inductionCooker.getoldisInduon() != inductionCooker.getisInduon())
        {
            inductionCooker.setoldisInduon();
            val = true;
        }
    }
    if (val)
    {
        char response[measureJson(ssedoc) + 1];
        serializeJson(ssedoc, response, sizeof(response));
        if (measureJson(ssedoc) > 2 && inductionCooker.getIsEnabled())
            SSEBroadcastJson(response, INDJSON);
    }
}

void miscSSE()
{
    JsonDocument ssedoc;
    ssedoc["host"] = mqtthost;
    ssedoc["port"] = mqttport;
    ssedoc["s_mqtt"] = mqtt_state;
    ssedoc["display"] = useDisplay;
    ssedoc["lang"] = selLang;
    if (startMDNS)
        ssedoc["mdns"] = nameMDNS;
    else
        ssedoc["mdns"] = 0;

    char response[measureJson(ssedoc) + 1];
    serializeJson(ssedoc, response, sizeof(response));
    SSEBroadcastJson(response, MISCJSON);
}

void handleChannel()
{
    uint8_t channel;
    for (channel = 0; channel < SSE_MAX_CHANNELS; channel++) // Find first free slot
    {
        if (!subscription[channel].clientIP)
        {
            break;
        }
    }
    subscription[channel].clientIP = server.client().remoteIP(); // get IP address of client
    subscription[channel].check = true;

    String SSEurl = F("http://");
    SSEurl += WiFi.localIP().toString();
    SSEurl += F(":");
    SSEurl += PORT;
    SSEurl += F("/rest/events/");
    SSEurl += channel;
    replyOKWithMsg(SSEurl.c_str());
}

void SSEKeepAlive()
{
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (!(subscription[i].clientIP))
        {
            continue;
        }
        if (subscription[i].client.connected())
        {
            // subscription[i].client.println("data: { \"TYPE\":\"KEEP-ALIVE\" }\n"); // Extra newline required by SSE standard
            // String alive = "event: alive\ndata: { \"type\":\"keep alive\", \"ip\":\"" + IPtoString(subscription[i].clientIP) + "\", \"channel\":\"" + i + "\"}\n";
            char alive[100];
            snprintf(alive, sizeof(alive), "event: alive\ndata: { \"type\":\"keep alive\", \"ip\":\"%s\", \"channel\":\"\"}\n", subscription[i].clientIP.toString().c_str(), i);
            subscription[i].client.println(alive);
        }
        else
        {
            subscription[i].keepAliveTimer.detach();
#ifdef ESP_IDF5
            subscription[i].client.clear();
#elif ESP32
            subscription[i].client.flush();
#elif ESP8266
            subscription[i].client.flush();
#endif
            subscription[i].client.stop();
            subscription[i].clientIP = INADDR_NONE;
            subscriptionCount--;
        }
    }
}

void SSEHandler(const uint8_t &channel)
{
    IPAddress clientIP = server.client().remoteIP(); // get IP address of client

    if (subscription[channel].check == true)
    {
        if (clientIP == subscription[channel].clientIP)
        {
            subscription[channel].client = server.client();
            subscription[channel].keepAliveTimer = Ticker();
            subscription[channel].check = false;
            subscriptionCount++;
        }
    }

    WiFiClient client = server.client();
    SSESubscription &s = subscription[channel];

    if (s.clientIP != client.remoteIP())
    { // IP addresses don't match, reject this client
        return handleNotFound();
    }
    client.setNoDelay(true);
    s.client = client; // capture SSE server client connection

    server.setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
    server.sendContent("HTTP/1.1 200 OK\nContent-Type: text/event-stream\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n");
    s.keepAliveTimer.attach(15.0, SSEKeepAlive); // Refresh time every 30s - WebUpdate benötigt bei langsamer Leitung über 60s
    initialSSE(channel);
}

void SSEBroadcastJson(const char *jsonValue, uint8_t typ)
{
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (!(subscription[i].clientIP))
            continue;

        WiFiClient client = subscription[i].client;
        String IPaddrstr = IPAddress(subscription[i].clientIP).toString();
        if (client)
        {
            char response[strlen(jsonValue) + 60];
            if (typ == SENSORJSON) // 0: sensors
            {
                snprintf(response, sizeof(response), "event: sensors\ndata: %s\nid: %lu\n\n", jsonValue, millis());
            }
            else if (typ == ACTORJSON) // 1: actors
            {
                snprintf(response, sizeof(response), "event: actors\ndata: %s\nid: %lu\n\n", jsonValue, millis());
            }
            else if (typ == INDJSON) // 2: induction
            {
                snprintf(response, sizeof(response), "event: ids\ndata: %s\nid: %lu\n\n", jsonValue, millis());
            }
            else if (typ == MISCJSON) // 3: misc System
            {
                snprintf(response, sizeof(response), "event: misc\ndata: %s\nid: %lu\n\n", jsonValue, millis());
            }
            else
            {
#ifdef ESP32
                DEBUG_ERROR("SYS", "unknown SSE broadcast type %d", typ);
#endif
                return;
            }
#ifdef ESP32
            DEBUG_VERBOSE("SYS", "%s", response);
#endif
            client.print(response);
        }
    }
}

void handleNotFound()
{
    if (loadFromLittlefs(server.uri()))
    {
        return;
    }

    String message = "Handle Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    replyNotFound(message);
}

void handleAll()
{
    const char *uri = server.uri().c_str();
    const char *restEvents = "/rest/events/";
    if (strncmp_P(uri, restEvents, strlen_P(restEvents)))
    {
        return handleNotFound();
    }
    uri += strlen_P(restEvents); // Skip the "/rest/events/" and get to the channel number
    unsigned int channel = atoi(uri);
    if (channel < SSE_MAX_CHANNELS)
        SSEHandler(channel);
    else
        handleNotFound();
}

void checkAliveSSE()
{
    IPAddress clientIP = server.client().remoteIP(); // get IP address of client
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (clientIP == subscription[i].clientIP)
        {
            replyOKWithMsg("1");
            return;
        }
    }
    replyOKWithMsg("-1");
}

void initialSSE(const uint8_t &val)
{
    // String alive = "event: alive\ndata: { \"type\":\"new SSE\", \"ip\":\"" + subscription[val].clientIP.toString() + "\", \"channel\":\"" + val + "\"}\n";
    char alive[100];
    snprintf(alive, sizeof(alive), "event: alive\ndata: {\"type\":\"new SSE\",\"ip\":\"%s\",\"channel\":\"%d\"}\n", subscription[val].clientIP.toString().c_str(), val);
    subscription[val].client.println(alive);
}
