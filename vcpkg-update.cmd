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
call .\bootstrap-vcpkg.bat
.\vcpkg remove --outdated --recurse
.\vcpkg.exe install proj[core,tiff]:x86-windows-static-md proj[core,tiff]:x64-windows-static-md tiff[core,lzma,zip]:x86-windows-static-md tiff[core,lzma,zip]:x64-windows-static-md || exit /b 1
popd