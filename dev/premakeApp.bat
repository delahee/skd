REM @echo off
..\tools\premake5.exe --roar=None --file=app_premake.lua --verbose vs2019
if %ERRORLEVEL% NEQ 0 PAUSE