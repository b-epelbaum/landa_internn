@echo off
setlocal
echo "************************************************************"
echo "********   Increase version  Script *****************"
echo "************************************************************"
echo ""
echo ""

set OLDDIR=%CD%

set RELTOOLDIR=.
set RELBUILDDIR=..\bin\Release
set RELCOMMON=..\include\common

rem // Save current directory and change to target directory
pushd %RELTOOLDIR%
rem // Save value of CD variable (current directory)
set TOOLDIR=%CD%
rem // Restore original directory
popd

rem // Save current directory and change to target directory
pushd %RELBUILDDIR%
rem // Save value of CD variable (current directory)
set BUILDDIR=%CD%
rem // Restore original directory
popd

rem // Save current directory and change to target directory
pushd %RELCOMMON%
rem // Save value of CD variable (current directory)
set COMMON=%CD%
rem // Restore original directory
popd

del *.log

echo "TOOLS DIRECTORY : " %TOOLDIR%
echo "BUILD DIRECTORY : " %BUILDDIR%
echo "COMMON DIRECTORY : " %COMMON%

echo ""
echo "*****   Generating version increment and buid   *******"
echo ""

%TOOLDIR%\sdVersionGenerator.exe %COMMON%\version.h  || exit /B 1 