@echo off

if not exist "zmac" (
  md zmac
)

if not exist "zmac\zmac.zip" (
  curl -o zmac\zmac.zip -JL http://48k.ca/zmac.zip || goto :ERROR
)

if not exist "zmac\zmac.exe" (
  pushd zmac
  tar -xf zmac.zip || goto :ERROR
  popd
)

zmac\zmac.exe zexall.src || goto :ERROR
copy zout\zexall.cim zexall.com || goto :ERROR

echo Build Status - SUCCEEDED
set /a errorno=0
goto :END


:ERROR
echo Abort by error.
echo Build Status - ERROR


:END
popd
exit /B %errorno%
