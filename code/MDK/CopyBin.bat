@echo off
@del /s /q ..\Bin\*.*
@for /F "tokens=3" %%i in ('findstr /c:"#define FIRMWARE_VERSION" "..\User\gisunlink.h"') do ( set VERSION=%%~i)
@set VERSION=%VERSION%.bin
@set "VERSION=%VERSION: =%"
@copy .\Objects\GSL_EEM_code.bin ..\Bin\%VERSION%
@echo on
