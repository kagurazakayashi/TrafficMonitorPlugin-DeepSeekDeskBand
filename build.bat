@ECHO OFF
REM ============================================================
REM  DeepSeekDeskBand - TrafficMonitor Plugin Build Script
REM  Automatically detects current platform (x86/x64).
REM ============================================================

SET MSBUILD=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe
SET SOLUTION=%~dp0DeepSeekDeskBand.sln
SET PLUGINDIR=C:\TrafficMonitor\plugins

IF NOT EXIST "%MSBUILD%" (
    ECHO ERROR: MSBuild not found at "%MSBUILD%"
    EXIT /B 1
)

IF NOT EXIST "%PLUGINDIR%" (
    ECHO Creating output directory: "%PLUGINDIR%"
    MKDIR "%PLUGINDIR%"
)

REM Detect current platform
IF /I "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    SET PLATFORM=x64
    SET DLLNAME=DeepSeekDeskBand64.dll
    SET DLLPATH=%~dp0x64\Release\DeepSeekDeskBand64.dll
) ELSE IF /I "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    SET PLATFORM=x64
    SET DLLNAME=DeepSeekDeskBand64.dll
    SET DLLPATH=%~dp0x64\Release\DeepSeekDeskBand64.dll
) ELSE (
    SET PLATFORM=x86
    SET DLLNAME=DeepSeekDeskBand.dll
    SET DLLPATH=%~dp0Release\DeepSeekDeskBand.dll
)

ECHO ============================================================
ECHO  Building DeepSeekDeskBand - Release ^| %PLATFORM%
ECHO ============================================================
"%MSBUILD%" "%SOLUTION%" /t:Rebuild /p:Configuration=Release /p:Platform=%PLATFORM% /v:minimal
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Build failed
    EXIT /B %ERRORLEVEL%
)

COPY /Y "%DLLPATH%" "%PLUGINDIR%\" >NUL
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Failed to copy %DLLNAME% to "%PLUGINDIR%"
    EXIT /B %ERRORLEVEL%
)

ECHO ============================================================
ECHO  Build completed.
ECHO    %PLUGINDIR%\%DLLNAME%  (%PLATFORM%)
ECHO ============================================================
EXIT /B 0
