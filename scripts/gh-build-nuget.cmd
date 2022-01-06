@echo off
setlocal enableextensions

if "%*" == "" (
  SET ARG=-gh
) ELSE (
  SET ARG=%* -gh
)

%0\..\..\nuspec\build.cmd %ARG%