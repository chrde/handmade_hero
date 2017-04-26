@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
set compilerFlags=/Zi /FC
set deps=user32.lib Gdi32.lib
set main=..\src\main.cpp
mkdir .\build
pushd .\build
cl %compilerFlags% %main% %deps%
popd