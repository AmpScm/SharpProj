@echo off
SETLOCAL ENABLEEXTENSIONS
PUSHD "%0\.."
SET BASEDIR=%CD%

IF NOT EXIST "..\vcpkg" (
  pushd "%BASEDIR%\.."
  git clone https://github.com/microsoft.com/vcpkg.git || exit /b 1
  popd %BASEDIR%
)

IF NOT EXIST "..\vcpkg\vcpkg.exe" (
  pushd "%BASEDIR%\..\vcpkg"
  git pull || exit /b 1
  CALL bootstrap-vcpkg.bat || exit /b 1
  popd
)

pushd ..\vcpkg
.\vcpkg.exe install "--overlay-ports=%BASEDIR%\vcpkg_ports" proj4:x64-windows-static-md proj4:x86-windows-static-md || exit /b 1
popd