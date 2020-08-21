@echo off
if "%1" == "" goto option
if not "%1" == "x86" if not "%1" == "x64" if not "%1" == "arm" if not "%1" == "arm64" goto build
call BuildWindows.bat lib xxMiniCRT %1 %2 %3 %4
call BuildWindows.bat dll freetype %1 %2 %3 %4
call BuildWindows.bat dll imgui %1 %2 %3 %4
call BuildWindows.bat dll xxGraphic %1 %2 %3 %4
call BuildWindows.bat dll xxGraphicPlus %1 %2 %3 %4
call BuildWindows.bat exe xxImGui %1 %2 %3 %4
exit
goto :eof

:option
echo BuildWindows.bat [Platform] [VisualStudioYear] [VisualStudioEdition] [VisualStudioVersion] [WindowsSDKVersion]
exit

:build
call vcvars.bat %3 %4 %5 %6
cd ..
mkdir lib >nul 2>nul
mkdir temp >nul 2>nul
mkdir temp/%2 >nul 2>nul
mkdir temp/%2/%3 >nul 2>nul
cd temp
cd %2
cd %3
cl.exe -MP -nologo -c -O2 -GS- -GR- /Zc:threadSafeInit- @../../../build/%2.sources
del ..\..\..\bin\%2.Release.%3.lib >nul 2>nul
if "%1" == "lib" (lib  -nologo *.obj /OUT:../../../lib/%2.Release.%3.lib)
if "%1" == "dll" (link -nologo *.obj /OUT:../../../bin/%2.Release.%3.dll ../../../bin/*.Release.%3.lib ../../../lib/*.Release.%3.lib chkstk.obj kernel32.lib user32.lib /dll)
if "%1" == "exe" (link -nologo *.obj /OUT:../../../bin/%2.Release.%3.exe ../../../bin/*.Release.%3.lib ../../../lib/*.Release.%3.lib chkstk.obj kernel32.lib user32.lib /LIBPATH:../../../lib)
cd ..
cd ..
cd ..
cd build
goto :eof
