pushd .
cd %CIRCA_HOME%
call scons cfsh
cd demos
call ..\cuttlefish\run.bat %*
popd
