#include "MQTTDevice.h"

#ifdef ESP32
int getTagLevel(const String &tagName)
{
  for (int i = 0; i < LOGS_COUNT; i++)
  {
    if (InnuTagLevel[i].tagName == tagName)
    {
      return InnuTagLevel[i].level;
    }
  }
  return INNU_NONE;
}

void setTagLevel(const String &tagName, int level)
{
  for (int i = 0; i < LOGS_COUNT; i++)
    if (InnuTagLevel[i].tagName == tagName)
    {
      InnuTagLevel[i].level = level;
      return;
    }
}

void saveLog()
{
  JsonDocument doc;
  doc["CFG"] = getTagLevel("CFG");
  doc["SEN"] = getTagLevel("SEN");
  doc["ACT"] = getTagLevel("ACT");
  doc["IND"] = getTagLevel("IND");
  doc["SYS"] = getTagLevel("SYS");
  doc["DIS"] = getTagLevel("DIS");

  File logFile = LittleFS.open(LOG_CFG, "w");
  if (!logFile)
  {
    DEBUG_ERROR("CFG", "error could not save log_cfg.json - permission denied");
    return;
  }
  serializeJson(doc, logFile);
  logFile.close();
  // DEBUG_INFO("CFG", "CFG: %d sen: %d act: %d ind: %d hlt: %d sys: %d mas: %d dis: %d", getTagLevel("CFG"), getTagLevel("SEN"), getTagLevel("ACT"), getTagLevel("IND"), getTagLevel("HLT"), getTagLevel("SYS"), getTagLevel("MAS"), getTagLevel("DIS"));
}

void readLog()
{
  File logFile = LittleFS.open(LOG_CFG, "r");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, logFile);
  if (error)
  {
    DEBUG_ERROR("SYS", "error could not read log_cfg: %s - JSON error %s", LOG_CFG, error.c_str());
    return;
  }
  logFile.close();
  setTagLevel("CFG", doc["CFG"] | 0);
  setTagLevel("SEN", doc["SEN"] | 0);
  setTagLevel("ACT", doc["ACT"] | 0);
  setTagLevel("IND", doc["IND"] | 0);
  setTagLevel("SUD", doc["SUD"] | 0);
  setTagLevel("HLT", doc["HLT"] | 0);
  setTagLevel("SYS", doc["SYS"] | 0);
  setTagLevel("MAS", doc["MAS"] | 0);
  setTagLevel("DIS", doc["DIS"] | 0);
  // DEBUG_INFO("CFG", "read logging CFG: %d sen: %d act: %d ind: %d hlt: %d sys: %d mas: %d dis: %d", getTagLevel("CFG"), getTagLevel("SEN"), getTagLevel("ACT"), getTagLevel("IND"), getTagLevel("HLT"), getTagLevel("SYS"), getTagLevel("MAS"), getTagLevel("DIS"));
}
#endif
