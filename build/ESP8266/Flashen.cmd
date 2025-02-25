@ECHO OFF
CLS
SET SCRIPT_LOCATION=%~dp0
cd %SCRIPT_LOCATION%
echo erase flash
esptool.exe --chip esp8266 erase_flash
echo Flash firmware and LittleFS 
esptool.exe --chip esp8266 --baud 921600 write_flash 0x000000 firmware.bin 0x200000 LittleFS.bin
echo ESC to quit
pause
exit
