@echo off
if "%1" == "" goto option
if "%1" == "all" goto all
if not "%1" == "x86" if not "%1" == "x64" if not "%1" == "arm" if not "%1" == "arm64" if not "%1" == "thumb" if not "%1" == "mipsii" if not "%1" == "mipsiii" if not "%1" == "mipsiv" if not "%1" == "mipsv" if not "%1" == "mips16" if not "%1" == "mips32" if not "%1" == "mips64" if not "%1" == "ppc" if not "%1" == "sh4" goto build
call BuildWindows.bat dll freetype %1 %2 %3 %4 %5
call BuildWindows.bat dll imgui %1 %2 %3 %4 %5
call BuildWindows.bat dll xxGraphic %1 %2 %3 %4 %5
call BuildWindows.bat dll xxGraphicPlus %1 %2 %3 %4 %5
call BuildWindows.bat exe xxImGui %1 %2 %3 %4 %5
call BuildWindows.bat plugin sample %1 %2 %3 %4 %5
call BuildWindows.bat plugin validator %1 %2 %3 %4 %5
goto :eof

:all
call BuildWindows.bat x86 %2 %3 %4 %5 %6
call BuildWindows.bat x64 %2 %3 %4 %5 %6
call BuildWindows.bat arm %2 %3 %4 %5 %6
call BuildWindows.bat arm64 %2 %3 %4 %5 %6
goto :eof

:option
echo BuildWindows.bat [Platform] [VisualStudioYear] [VisualStudioEdition] [VisualStudioVersion] [WindowsSDKVersion]
echo BuildWindows.bat all 2015
echo BuildWindows.bat x86 2017 Community 14.13.26128
echo BuildWindows.bat x64 2019 Community 14.24.28314 10.0.19041.0
goto :eof

:build
set BACKUP_PATH=%PATH%
set BACKUP_INCLUDE=%INCLUDE%
set BACKUP_LIB=%LIB%
set BACKUP_CL=%CL%
set BACKUP_LINKOPT=%LINKOPT%
call vcvars.bat %3 %4 %5 %6 %7
cd ..
mkdir bin >nul 2>nul
mkdir bin\plugin >nul 2>nul
mkdir lib >nul 2>nul
mkdir temp >nul 2>nul
mkdir temp\%2 >nul 2>nul
mkdir temp\%2\%3 >nul 2>nul
cd temp
cd %2
cd %3
cl.exe /MP /nologo /c /O2 /GS- /GR- /fp:fast /Z7 /wd4819 /std:c++latest /utf-8 @..\..\..\build\%2.sources
del ..\..\..\bin\%2.Release.%3.lib >nul 2>nul
del ..\..\..\bin\plugin\%2.Release.%3.lib >nul 2>nul
set LINKOPT=%LINKOPT% ..\..\..\bin\*.Release.%3.lib ..\..\..\lib\*.Release.%3.lib /OPT:REF /OPT:ICF
if "%1" == "lib"    (lib  /nologo *.obj /OUT:..\..\..\lib\%2.Release.%3.lib)
if "%1" == "dll"    (link /nologo *.obj /OUT:..\..\..\bin\%2.Release.%3.dll %LINKOPT% /dll)
if "%1" == "exe"    (link /nologo *.obj /OUT:..\..\..\bin\%2.Release.%3.exe %LINKOPT% /LIBPATH:..\..\..\lib)
if "%1" == "plugin" (link /nologo *.obj /OUT:..\..\..\bin\plugin\%2.Release.%3.dll ..\..\..\bin\plugin\*.Release.%3.lib %LINKOPT% /dll)
set LINKOPT=
cd ..
cd ..
cd ..
cd build
set PATH=%BACKUP_PATH%
set INCLUDE=%BACKUP_INCLUDE%
set LIB=%BACKUP_LIB%
set CL=%BACKUP_CL%
set LINKOPT=%BACKUP_LINKOPT%
goto :eof
