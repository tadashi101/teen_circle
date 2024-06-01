@echo off

set tools_folder=%USERPROFILE%\AppData\Local\Arduino15\packages\esp32\tools
echo %tools_folder%

rem make binary for spiffs(default 4MB with spiffs)
%tools_folder%\mkspiffs\0.2.3\mkspiffs.exe ^
  -c .\data ^
  -s 0x160000 ^
  ..\..\..\build\led_clock.spiffs.bin

rem flash spiffs
%tools_folder%\esptool_py\4.5.1\esptool.exe ^
    --chip esp32 ^
    --baud 921600  ^
    --before default_reset ^
    --after hard_reset ^
    write_flash ^
    -z ^
    --flash_mode dio ^
    --flash_freq 80m ^
    --flash_size 4MB ^
    0x290000 ^
    ..\..\..\build\led_clock.spiffs.bin

pause