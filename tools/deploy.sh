rm -rf deploy
mkdir deploy

mkdir deploy/bin

cp -R ptc/* deploy
cp -R SDL_deps/bin/* deploy/bin
cp build/bin/cfish.exe deploy/bin

cp /c/Program\ Files/Microsoft\ Visual\ Studio\ 8/VC/redist/x86/Microsoft.VC80.CRT/*dll deploy/bin
