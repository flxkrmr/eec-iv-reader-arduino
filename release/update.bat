@echo off
set /p port=<port.txt

if %port% == COMX (
    echo Arduino PORT not set
    echo Please set Arduino PORT in port.txt to one of the following devices:
    mode
    pause
    exit
)

set version=0.3.3
echo Arduino PORT set to %port%
echo Upload Software Version %version%
avrdude-v7.1-windows-x64\avrdude.exe -v -p atmega328p -C avrdude-v7.1-windows-x64\avrdude.conf -c arduino -b 115200 -D -P %port% -U flash:w:firmware-%version%.hex:i
pause