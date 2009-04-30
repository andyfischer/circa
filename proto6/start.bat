set PATH=%PATH%;%~dp0\..\SDL_deps\bin

pushd .

cd %~dp0
app.exe main.ca

popd
