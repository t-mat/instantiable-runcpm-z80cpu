@echo off
setlocal enabledelayedexpansion
set /a errorno=1
for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"
cd /d "%~dp0"

set EXE_PATH=.
set EXE_ARGS=

call .\build.cmd || goto :ERROR

set "EXE="
set "EXE_DIR="
for /f "delims=" %%a in ('where.exe /f "%EXE_PATH%:*.exe"') do (
  set "EXE=%%a"
  set "EXE_DIR=%%~dpa"
)
if "%EXE%" == "" (
  echo %~n0 : ERROR - Can't find executable file in %EXE_PATH%
  set /a errorno=1
  goto :ERROR
)

@rem cd /d "%EXE_DIR%"

echo.
echo %EXE% %EXE_ARGS%
     %EXE% %EXE_ARGS% || goto :FAIL

echo %~n0 : Status=%_ESC%[92m OK %_ESC%[0m
set /a errorno=0
goto :END

:FAIL
echo %~n0 : Status=%_ESC%[91m FAIL %_ESC%[0m

:ERROR
echo %~n0 : %_ESC%[91m ERROR %_ESC%[0m
set /a errorno=1

:END
exit /B %errorno%
