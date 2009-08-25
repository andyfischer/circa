@ECHO OFF

pushd .
cd %CIRCA_HOME%
scons plastic && cd plastic\demos && call ..\run.bat %1.ca
popd
