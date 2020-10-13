@echo off
set Platform=x64
set VisualStudioYear=2019
for /D %%s in ("C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\*") do set VisualStudioEdition=%%~nxs
for /D %%s in ("C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%\VC\Tools\MSVC\*") do set VisualStudioVersion=%%~nxs
for /D %%s in ("C:\Program Files (x86)\Windows Kits\10\Include\*") do set WindowsSDKVersion=%%~nxs
if not "%1" == "" set Platform=%1
if not "%2" == "" set VisualStudioYear=%2
if not "%3" == "" set VisualStudioEdition=%3
if not "%4" == "" set VisualStudioVersion=%4
if not "%5" == "" set WindowsSDKVersion=%5

if not %VisualStudioYear% == 2015 goto newVC

:oldVC
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
goto winSDK

:newVC
set VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\%VisualStudioYear%\%VisualStudioEdition%
set VCINSTALLDIR=%VSINSTALLDIR%\VC\Tools\MSVC\%VisualStudioVersion%

set PATH="%VCINSTALLDIR%\bin\Hostx64\%Platform%";%PATH%
set INCLUDE="%VCINSTALLDIR%\include";%INCLUDE%
set LIB="%VCINSTALLDIR%\lib\%Platform%";%LIB%
goto winSDK

:winSDK
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\10

set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\shared";%INCLUDE%
set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\ucrt";%INCLUDE%
set INCLUDE="%WindowsSdkDir%\include\%WindowsSDKVersion%\um";%INCLUDE%
set LIB="%WindowsSdkDir%\lib\%WindowsSDKVersion%\ucrt\%Platform%";%LIB%
set LIB="%WindowsSdkDir%\lib\%WindowsSDKVersion%\um\%Platform%";%LIB%

if "%1" == "" echo vcvars.bat [Platform] [VisualStudioYear] [VisualStudioEdition] [VisualStudioVersion] [WindowsSDKVersion]
if "%1" == "" echo For example:
if "%1" == "" echo   vcvars.bat x64 2015
if "%1" == "" echo   vcvars.bat x64 2017 Community 14.13.26128
if "%1" == "" echo   vcvars.bat x64 2019 Community 14.24.28314 10.0.19041.0

set CL=/I"%VCINSTALLDIR%\include" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\shared" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\ucrt" /I"%WindowsSdkDir%\include\%WindowsSDKVersion%\um"
