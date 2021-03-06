@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" x64
set commonCompilerFlags=-MTd -Gm- -GR- -EHa- -Od -Oi -fp:fast -Z7 -FC -nologo
set compilerFlags=%commonCompilerFlags% -Femain -Fmmain.map
set compilerFlagsForGame=%commonCompilerFlags% -Fehandmade -Fmhandmade.map  
set commonLinkerFlags=/link -incremental:no -opt:ref
set linkerOptions=%commonLinkerFlags% -subsystem:windows,5.2 user32.lib Gdi32.lib Winmm.lib
set linkerOptionsForGame=/LD %commonLinkerFlags% -PDB:handmade_%random%.pdb /EXPORT:gameGetSoundSamples /EXPORT:gameUpdateAndRender
set allowedWarnings=-W4 -WX -wd4201 -wd4100 -wd4189
set compileOptions=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_WIN=1 %allowedWarnings%
set main=..\src\main.cpp
set game=..\src\handmade.cpp
IF NOT EXIST .\build mkdir .\build
pushd .\build
del *.pdb > NUL 2> NUL
cl %compileOptions% %compilerFlagsForGame% %game% %linkerOptionsForGame%
cl %compileOptions% %compilerFlags% %main% %linkerOptions%
popd