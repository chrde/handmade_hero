@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" x64
set compilerFlags=-MT -Gm- -GR- -EHa- -Od- -Oi- -Zi -FC -Femain -nologo
set linkerOptions=/link -subsystem:windows,5.2
set allowedWarnings=-W4 -WX -wd4201 -wd4100 -wd4189
set compileOptions=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 %allowedWarnings%
set deps=user32.lib Gdi32.lib
set main=..\src\main.cpp
IF NOT EXIST .\build mkdir .\build
pushd .\build
cl %compileOptions% %compilerFlags% %main% %linkerOptions% %deps%
popd