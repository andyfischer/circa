pushd .
cd %CIRCA_HOME%
scons cfish && cd demos && ..\cuttlefish\run.bat %*
popd
