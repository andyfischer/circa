@ECHO OFF

pushd .
cd %CIRCA_HOME%
scons plastic && cd plastic\demos && call ..\plastic\run.bat %1.ca
popd
