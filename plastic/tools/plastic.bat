@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

:: Include SDL binaries
set PATH=%PATH%;%~dp0\..\..\SDL_deps\bin;%~dp0\..\deps\bin

%~dp0\..\..\build\bin\plas.exe %*
