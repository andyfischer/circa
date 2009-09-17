@ECHO OFF

pushd .
cd %CIRCA_HOME%
scons plastic && cd plastic\demos && call ..\..\tools\plastic.bat %1 %2 %3 %4 %5
popd
