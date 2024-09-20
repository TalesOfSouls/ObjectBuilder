cls

IF NOT EXIST ..\build mkdir ..\build
IF NOT EXIST ..\build\objbuilder mkdir ..\build\objbuilder

if not defined DevEnvDir (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - previous bat call failed.
    exit /b 1
)

cd ..\build\objbuilder
del *.pdb > NUL 2> NUL
del *.idb > NUL 2> NUL
cd ..\..\ObjectBuilder

REM Use /showIncludes for include debugging

REM Create main program
cl ^
    /MT /nologo /Gm- /GR- /EHsc /Od /Oi /WX /W4 /FC /Z7 /wd4201 /wd4100 ^
    /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /std:c++20 ^
    /D WIN32 /D _WINDOWS /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS ^
    /Fo"../build/objbuilder/" /Fe"../build/objbuilder/obj_builder.exe" /Fd"../build/objbuilder/obj_builder.pdb" /Fm"../build/objbuilder/obj_builder.map" ^
    "obj_builder.cpp"^
    /link /INCREMENTAL:no ^
    /SUBSYSTEM:CONSOLE /MACHINE:X64 ^
    kernel32.lib user32.lib gdi32.lib winmm.lib