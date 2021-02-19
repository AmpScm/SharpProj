@echo off
setlocal enableextensions
SET PROJ_VER=%1
SET SHARPPROJ_VER=%2

if "%SHARPPROJ_VER%" == "" (
  echo "%0 <proj-ver> <sharpproj-ver>"
  exit /b 1
)

pushd ..\src
msbuild /m /p:Configuration=Release /p:Platform=x86 /p:ForceAssemblyVersion=%SHARPPROJ_VER%  /v:m /nologo || exit /B 1
msbuild /m /p:Configuration=Release /p:Platform=x64 /p:ForceAssemblyVersion=%SHARPPROJ_VER% /v:m /nologo || exit /B 1
popd
if NOT EXIST ".\obj\." mkdir obj
if NOT EXIST ".\bin\." mkdir bin
copy ..\..\vcpkg\installed\x86-windows-static-md\share\proj4\*.db obj\ || exit /B 1
copy ..\..\vcpkg\installed\x86-windows-static-md\share\proj4\*.ini obj\ || exit /B 1

CALL :xmlpoke SharpProj.Database.nuspec //nu:metadata/nu:version %PROJ_VER% || EXIT /B 1

CALL :xmlpoke SharpProj.Core.nuspec //nu:metadata/nu:version %SHARPPROJ_VER% || EXIT /B 1

CALL :xmlpoke SharpProj.nuspec //nu:metadata/nu:version %SHARPPROJ_VER% || EXIT /B 1
CALL :xmlpoke SharpProj.nuspec "//nu:dependency[@id='SharpProj.Core']/@version" "[%SHARPPROJ_VER%]" || EXIT /B 1
CALL :xmlpoke SharpProj.nuspec "//nu:dependency[@id='SharpProj.Database']/@version" "[%PROJ_VER%]" || EXIT /B 1


CALL :xmlpoke SharpProj.NetTopologySuite.nuspec //nu:metadata/nu:version %SHARPPROJ_VER% || EXIT /B 1
CALL :xmlpoke SharpProj.NetTopologySuite.nuspec "//nu:dependency[@id='SharpProj.Core']/@version" "[%SHARPPROJ_VER%]" || EXIT /B 1

nuget pack SharpProj.Database.nuspec -version %PROJ_VER% -OutputDirectory bin || exit /B 1
nuget pack SharpProj.Core.nuspec -version %SHARPPROJ_VER% -OutputDirectory bin || exit /B 1
nuget pack SharpProj.nuspec -version %SHARPPROJ_VER% -OutputDirectory bin || exit /B 1
nuget pack SharpProj.NetTopologySuite.nuspec -version %SHARPPROJ_VER% -OutputDirectory bin || exit /B 1
echo "--done--"
goto :eof

:xmlpoke
msbuild /nologo /v:m xmlpoke.build "/p:File=%1" "/p:XPath=%2" "/p:Value=%3"
exit /b 0
