@echo off
SET PROJ_VER=%1
SET SHARPPROJ_VER=%2

if "%SHARPPROJ_VER%" == "" (
  echo "%0 <proj-ver> <sharpproj-ver>"
  exit /b 1
)

setlocal enableextensions
pushd ..\src
msbuild /m /p:Configuration=Release /p:Platform=x86 /p:ForceAssemblyVersion=%SHARPPROJ_VER% || exit /B 1
msbuild /m /p:Configuration=Release /p:Platform=x64 /p:ForceAssemblyVersion=%SHARPPROJ_VER% || exit /B 1
popd
if NOT EXIST ".\obj\." mkdir obj
if NOT EXIST ".\bin\." mkdir bin
copy ..\..\vcpkg\installed\x86-windows-static-md\share\proj4\*.db obj\ || exit /B 1
copy ..\..\vcpkg\installed\x86-windows-static-md\share\proj4\*.ini obj\ || exit /B 1
nuget pack SharpProj.Database.nuspec -version %PROJ_VER% -OutputDirectory bin
nuget pack SharpProj.Core.nuspec -version %SHARPPROJ_VER% -OutputDirectory bin