@ECHO off
cls
echo start firmware package ESP-IDF4

set framework=C:\Arduino\git\MQTTDevice32pIO

copy %framework%\.pio\build\ESP32_IDF4\firmware.bin %framework%\build\ESP32-IDF4\firmware.bin /Y
copy %framework%\.pio\build\ESP32_IDF4\Littlefs.bin %framework%\build\ESP32-IDF4\Littlefs.bin /Y
copy %framework%\.pio\build\ESP32_IDF4\bootloader.bin %framework%\build\ESP32-IDF4\bootloader.bin /Y
copy %framework%\.pio\build\ESP32_IDF4\partitions.bin %framework%\build\ESP32-IDF4\partitions.bin /Y
copy %homepath%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin %framework%\build\ESP32-IDF4\boot_app0.bin /Y

IF EXIST %framework%\MQTTDevice32.zip del %framework%\MQTTDevice32.zip
cd %framework%\build\ESP32-IDF4\

"C:\Program Files\7-Zip\7z.exe" a %framework%\MQTTDevice32.zip *.bin esptool.exe flashen.cmd

cd %framework%\
echo firmware package ESP-IDF4 complete.
