@ECHO OFF
CLS
SET SCRIPT_LOCATION=%~dp0
cd %SCRIPT_LOCATION%
echo erase flash
esptool.exe --chip esp32 erase_flash
echo Flash firmware and LittleFS 
esptool.exe --chip esp32 --baud 921600 --before default_reset --after hard_reset write_flash 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin 0x350000 LittleFS.bin
echo ESC to quit
pause
exit
