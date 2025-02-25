@ECHO off
cls
echo start firmware package ESP8266
set framework=C:\Arduino\git\MQTTDevice32pIO

IF EXIST %framework%\build\ESP8266\LittleFS.bin del %framework%\build\ESP8266\LittleFS.bin
IF EXIST %framework%\build\ESP8266\firmware.bin del %framework%\build\ESP8266\firmware.bin
IF EXIST %framework%\MQTTDevice.zip del %framework%\MQTTDevice.zip

copy %framework%\.pio\build\ESP8266\firmware.bin %framework%\build\ESP8266\firmware.bin /Y
copy %framework%\.pio\build\ESP8266\LittleFS.bin %framework%\build\ESP8266\LittleFS.bin /Y

cd %framework%\build\ESP8266\
"C:\Program Files\7-Zip\7z.exe" a %framework%\MQTTDevice.zip *.bin esptool.exe flashen.cmd

cd %framework%\
echo firmware package ESP8266 complete.
