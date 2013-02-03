@echo off
cd /d "%~dp0"
if not exist build mkdir build
cd build
del /q /s *
if not exist blake2 mkdir blake2
::cd blake2
::"%MSYS%\bin\bash.exe" --login -c "cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -G Ninja ../../blake2 && ninja"
::cd ..
call "%VS110COMNTOOLS%..\..\vc\vcvarsall.bat"
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
::cmake -G "Visual Studio 10" ..
pause
