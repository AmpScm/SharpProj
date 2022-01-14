@echo off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

SET CACHE=%0\..\gh.cache.bat
SET RSPFILE=%0\..\msbuild-version.rsp

if NOT EXIST "%1" (
  echo "Usage: %0 <proj.h> <buildnr>"
  goto :eof
)

echo @echo off > %CACHE%

for /F "usebackq tokens=2,3" %%i in (`"type %1 |findstr /C:_VERSION_"`) do (
  SET %%i=%%j
  echo SET %%i=%%j>> %CACHE%
) 

pushd %0\..
FOR /F "usebackq" %%i in (`git rev-parse HEAD`) do (
  SET GIT_SHA=%%i
)
popd

set    SHARPPROJ_MAJOR=%PROJ_VERSION_MAJOR%
set /a SHARPPROJ_MINOR=%PROJ_VERSION_MINOR% * 1000 + %PROJ_VERSION_PATCH%
set    SHARPPROJ_PATCH=%2

echo Found PROJ %PROJ_VERSION_MAJOR%.%PROJ_VERSION_MINOR%.%PROJ_VERSION_PATCH% from header
echo Prepare building SharpProj %PROJ_VERSION_MAJOR%.%SHARPPROJ_MINOR%.%2

(
  echo SET SHARPPROJ_MAJOR=%SHARPPROJ_MAJOR%
  echo SET SHARPPROJ_MINOR=%SHARPPROJ_MINOR%
  echo SET SHARPPROJ_PATCH=%SHARPPROJ_PATCH%
  echo SET GIT_SHA=%GIT_SHA%
) >> %CACHE%

(
  REM For SDK Projects
  echo /p:Version=%SHARPPROJ_MAJOR%.%SHARPPROJ_MINOR%.%SHARPPROJ_PATCH%
  echo /p:Company="SharpProj Project, powered by AmpScm, QQn & GitHub"
  echo /p:Copyright="Apache 2.0 licensed. See https://github.com/ampscm/SharpProj"
  echo /p:InformationalVersion=%SHARPPROJ_MAJOR%.%SHARPPROJ_MINOR%.%SHARPPROJ_PATCH%-%GIT_SHA%
  
  REM For SharpSvn.AnnotateAssembly
  echo /p:ForceAssemblyVersion=%SHARPPROJ_MAJOR%.%SHARPPROJ_MINOR%.%SHARPPROJ_PATCH%
  echo /p:ForceAssemblyCompany="SharpProj Project, powered by AmpScm, QQn & GitHub"
  echo /p:ForceAssemblyCopyright="Apache 2.0 licensed. See https://github.com/AmpScm/SharpProj"
  echo /p:ForceAssemblyInformationalVersion=%SHARPPROJ_MAJOR%.%SHARPPROJ_MINOR%.%SHARPPROJ_PATCH%-%GIT_SHA%
  
  REM For scripting
  echo /p:BuildBotBuild=true
  echo /p:RestoreForce=true
  echo /p:RestorePackagesConfig=true
) > %RSPFILE%
