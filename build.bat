echo off
fsutil reparsepoint query third_party > ~tmp
if %errorlevel% == 0 (
	echo 0
	echo "link of third_party exist"
)
if %errorlevel% == 1 (
	echo "create link for third_party"
	mklink /d third_party ..\third_party
)
del ~tmp

if not exist "build" (
	mkdir build
)
cd build
if not exist "win" (
	mkdir win
)
cd win
cmake -G"Visual Studio 16 2019" ../../ 
cd ..\..\

