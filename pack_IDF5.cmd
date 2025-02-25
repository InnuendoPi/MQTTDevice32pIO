@ECHO off
echo start firmware package ESP32-IDF5

set framework=C:\Arduino\git\MQTTDevice32pIO

copy %framework%\.pio\build\ESP32_IDF5\firmware.bin %framework%\build\ESP32-IDF5\firmware.bin /Y
copy %framework%\.pio\build\ESP32_IDF5\Littlefs.bin %framework%\build\ESP32-IDF5\Littlefs.bin /Y
copy %framework%\.pio\build\ESP32_IDF5\bootloader.bin %framework%\build\ESP32-IDF5\bootloader.bin /Y
copy %framework%\.pio\build\ESP32_IDF5\partitions.bin %framework%\build\ESP32-IDF5\partitions.bin /Y
copy %homepath%\.platformio\packages\framework-arduinoespressif32@src-702d0f93023d86e22d8ef62aa333f0b7\tools\partitions\boot_app0.bin %framework%\build\ESP32-IDF5\boot_app0.bin /Y

IF EXIST %framework%\MQTTDevice32pIO.zip del %framework%\MQTTDevice32pIO.zip
cd %framework%\build\ESP32-IDF5\

"C:\Program Files\7-Zip\7z.exe" a %framework%\MQTTDevice32pIO.zip *.bin esptool.exe flashen.cmd

cd %framework%\
echo firmware package ESP32-IDF5 complete.
