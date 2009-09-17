@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

:: Include SDL binaries
set PATH=%PATH%;%~dp0\..\SDL_deps\bin

%~dp0\..\build\bin\plas.exe %1 %2 %3 %4 %5
