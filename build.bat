@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" x64
set compilerFlags=-MT -Gm- -GR- -EHa- -Od -Oi -Z7 -FC -Femain -nologo -Fmmain.map
set compilerFlagsForGame=-MT -Gm- -GR- -EHa- -Od -Oi -Z7 -FC -Fehandmade -nologo -Fmhandmade.map
set linkerOptions=/link -subsystem:windows,5.2 -opt:ref user32.lib Gdi32.lib Winmm.lib
set linkerOptionsForGame=/link /DLL /EXPORT:gameGetSoundSamples /EXPORT:gameUpdateAndRender
set allowedWarnings=-W4 -WX -wd4201 -wd4100 -wd4189
set compileOptions=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_WIN=1 %allowedWarnings%
set main=..\src\main.cpp
set game=..\src\handmade.cpp
IF NOT EXIST .\build mkdir .\build
pushd .\build
cl %compileOptions% %compilerFlagsForGame% %game% %linkerOptionsForGame%
cl %compileOptions% %compilerFlags% %main% %linkerOptions%
popd