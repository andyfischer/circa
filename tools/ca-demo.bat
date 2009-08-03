@ECHO OFF

pushd .
cd %CIRCA_HOME%
scons cfish && cd demos && call ..\cuttlefish\run.bat %1.ca
popd
