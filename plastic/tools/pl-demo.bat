@ECHO OFF

pushd .
cd %CIRCA_HOME%
scons plastic && cd plastic\demos && call ..\tools\plastic.bat %1.ca
popd
