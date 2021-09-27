@echo off
set Platform=x64
set VisualStudioYear=2019
set VisualStudioEdition=Community
set VisualStudioVersion=14.24.28314
set WindowsSDKVersion=10.0.19041.0
for /D %%s in ("C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\*") do set VisualStudioEdition=%%~nxs
for /D %%s in ("C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%\VC\Tools\MSVC\*") do set VisualStudioVersion=%%~nxs
for /D %%s in ("C:\Program Files (x86)\Windows Kits\10\Include\*") do set WindowsSDKVersion=%%~nxs
if not "%1" == "" set Platform=%1
if not "%2" == "" set VisualStudioYear=%2
if not "%3" == "" set VisualStudioEdition=%3
if not "%4" == "" set VisualStudioVersion=%4
if not "%5" == "" set WindowsSDKVersion=%5

if %VisualStudioYear% == 2022 goto vc2022
if %VisualStudioYear% == 2019 goto vc2017_2019
if %VisualStudioYear% == 2017 goto vc2017_2019
if %VisualStudioYear% == XBOX360 goto xbox360
if %VisualStudioYear% == WINCE700 goto wince
if %VisualStudioYear% == WINCE600 goto wince

:xbox360

set VSINSTALLDIR=C:\XBOX360
set VCINSTALLDIR=%VSINSTALLDIR%\BIN

set PATH="%VCINSTALLDIR%\win32";%PATH%
set INCLUDE="%VSINSTALLDIR%\include\xbox";%INCLUDE%
set LIB="%VSINSTALLDIR%\lib\xbox";%LIB%
set CL=%CL% /I"%VSINSTALLDIR%\include\xbox"
set LINKOPT=/NODEFAULTLIB /LIBPATH:"%VSINSTALLDIR%\lib\xbox" %LINKOPT%
goto :eof

:wince

if "%Platform%" == "arm" set PlatformBin=arm
if "%Platform%" == "thumb" set PlatformBin=arm
if "%Platform%" == "mipsii" set PlatformBin=mips
if "%Platform%" == "mipsiii" set PlatformBin=mips
if "%Platform%" == "mipsiv" set PlatformBin=mips
if "%Platform%" == "mipsv" set PlatformBin=mips
if "%Platform%" == "mips16" set PlatformBin=mips
if "%Platform%" == "mips32" set PlatformBin=mips
if "%Platform%" == "mips64" set PlatformBin=mips
if "%Platform%" == "sh4" set PlatformBin=sh
if "%Platform%" == "x86" set PlatformBin=x86

if "%Platform%" == "arm" set PlatformLib=arm
if "%Platform%" == "thumb" set PlatformLib=thumb
if "%Platform%" == "mipsii" set PlatformLib=mipsii
if "%Platform%" == "mipsiii" set PlatformLib=mipsii_fp
if "%Platform%" == "mipsiv" set PlatformLib=mipsii_fp
if "%Platform%" == "mipsv" set PlatformLib=mipsii
if "%Platform%" == "mips16" set PlatformLib=mips16
if "%Platform%" == "mips32" set PlatformLib=mipsii
if "%Platform%" == "mips64" set PlatformLib=mipsii_fp
if "%Platform%" == "sh4" set PlatformLib=sh4
if "%Platform%" == "x86" set PlatformLib=x86

if "%Platform%" == "thumb" set CL=/QRthumb2
if "%Platform%" == "mipsii" set CL=/QMmips2
if "%Platform%" == "mipsiii" set CL=/QMmips3
if "%Platform%" == "mipsiv" set CL=/QMmips4
if "%Platform%" == "mipsv" set CL=/QMmips5
if "%Platform%" == "mips16" set CL=/QMmips16
if "%Platform%" == "mips32" set CL=/QMmips32
if "%Platform%" == "mips64" set CL=/QMmips64

set VSINSTALLDIR=C:\%VisualStudioYear%
set VCINSTALLDIR=%VSINSTALLDIR%\BIN

set PATH="%VCINSTALLDIR%\%PlatformBin%";%PATH%
set INCLUDE="%VSINSTALLDIR%\inc";%INCLUDE%
set LIB="%VSINSTALLDIR%\lib\%PlatformLib%";%LIB%
set CL=%CL% /I"%VSINSTALLDIR%\inc"
set LINKOPT=coredll.lib /NOENTRY /NODEFAULTLIB /SUBSYSTEM:WINDOWSCE /LIBPATH:"%VSINSTALLDIR%\lib\%PlatformLib%" %LINKOPT%
goto :eof

:vc2015

if "%Platform%" == "x86" set PlatformBin=amd64_x86
if "%Platform%" == "x64" set PlatformBin=amd64
if "%Platform%" == "arm" set PlatformBin=amd64_arm

if "%Platform%" == "x86" set PlatformLib=
if "%Platform%" == "x64" set PlatformLib=amd64
if "%Platform%" == "arm" set PlatformLib=arm

set VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0
set VCINSTALLDIR=%VSINSTALLDIR%\VC

set PATH="%VCINSTALLDIR%\bin\%PlatformBin%";%PATH%
set INCLUDE="%VCINSTALLDIR%\include";%INCLUDE%
set LIB="%VCINSTALLDIR%\lib\%PlatformLib%";%LIB%
set LINKOPT=kernel32.lib user32.lib /DEBUG /LIBPATH:"%VCINSTALLDIR%\lib\%PlatformLib%" %LINKOPT%
goto winSDK

:vc2017_2019
set VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%
set VCINSTALLDIR=%VSINSTALLDIR%\VC\Tools\MSVC\%VisualStudioVersion%

set PATH="%VCINSTALLDIR%\bin\Hostx64\%Platform%";%PATH%
set INCLUDE="%VCINSTALLDIR%\include";%INCLUDE%
set LIB="%VCINSTALLDIR%\lib\%Platform%";%LIB%
set LINKOPT=kernel32.lib user32.lib /DEBUG /LIBPATH:"%VCINSTALLDIR%\lib\%Platform%" %LINKOPT%
goto winSDK

:vc2022
for /D %%s in ("C:\Program Files\Microsoft Visual Studio\%VisualStudioYear%\*") do set VisualStudioEdition=%%~nxs
for /D %%s in ("C:\Program Files\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%\VC\Tools\MSVC\*") do set VisualStudioVersion=%%~nxs
if not "%3" == "" set VisualStudioEdition=%3
if not "%4" == "" set VisualStudioVersion=%4

set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%
set VCINSTALLDIR=%VSINSTALLDIR%\VC\Tools\MSVC\%VisualStudioVersion%

set PATH="%VCINSTALLDIR%\bin\Hostx64\%Platform%";%PATH%
set INCLUDE="%VCINSTALLDIR%\include";%INCLUDE%
set LIB="%VCINSTALLDIR%\lib\%Platform%";%LIB%
set LINKOPT=kernel32.lib user32.lib /DEBUG /LIBPATH:"%VCINSTALLDIR%\lib\%Platform%" %LINKOPT%
goto winSDK

:winSDK
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\10

set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\shared";%INCLUDE%
set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\ucrt";%INCLUDE%
set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\um";%INCLUDE%
set LIB="%WindowsSdkDir%\lib\%WindowsSDKVersion%\ucrt\%Platform%";%LIB%
set LIB="%WindowsSdkDir%\lib\%WindowsSDKVersion%\um\%Platform%";%LIB%
set LINKOPT=/LIBPATH:"%WindowsSdkDir%\lib\%WindowsSDKVersion%\ucrt\%Platform%" %LINKOPT%
set LINKOPT=/LIBPATH:"%WindowsSdkDir%\lib\%WindowsSDKVersion%\um\%Platform%" %LINKOPT%

if "%1" == "" echo vcvars.bat [Platform] [VisualStudioYear] [VisualStudioEdition] [VisualStudioVersion] [WindowsSDKVersion]
if "%1" == "" echo For example:
if "%1" == "" echo   vcvars.bat x64 2015
if "%1" == "" echo   vcvars.bat x64 2017 Community 14.13.26128
if "%1" == "" echo   vcvars.bat x64 2019 Community 14.24.28314 10.0.19041.0

set CL=/I"%VCINSTALLDIR%\include" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\shared" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\ucrt" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\um"
