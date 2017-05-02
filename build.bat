@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" x64
set compilerFlags=/Zi /FC /Femain
set deps=user32.lib Gdi32.lib
set main=..\src\main.cpp
IF NOT EXIST .\build mkdir .\build
pushd .\build
cl %compilerFlags% %main% %deps%
popd