pushd .
cd %CIRCA_HOME%
call scons cfish
cd demos
call ..\cuttlefish\run.bat %*
popd
