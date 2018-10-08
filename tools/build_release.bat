@echo off
setlocal

echo "*******************************************************************************"
echo "********   				June QCS Build  Script 
echo "*******************************************************************************"
echo ""
echo ""


set OLDDIR=%CD%

set RELTOOLDIR=.
set RELBUILDDIR=..\bin\Release
set RELSLNDIR=..\

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

rem // Save current directory and change to target directory
pushd %RELSLNDIR%
rem // Save value of CD variable (current directory)
set SLNDIR=%CD%
rem // Restore original directory
popd

del *.log

echo "TOOLS DIRECTORY : " %TOOLDIR%
echo "BUILD DIRECTORY : " %BUILDDIR%
echo "COMMON DIRECTORY : " %COMMON%
echo "SOLUTION DIRECTORY : " %SLNDIR%

echo "--------------------------------------------"
echo "*****   		Building solution...   *******"
echo "--------------------------------------------"

for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set InstallDir=%%i
)

if exist "%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
  "%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" %SLNDIR%\JuneQCS.sln /p:Configuration=Release /t:Clean,Build 
)

echo "--------------------------------------------"
echo "*****   		Building finiished     *******"
echo "--------------------------------------------"

echo/

del %BUILDDIR%\*.lib
del %BUILDDIR%\*.iobj
del %BUILDDIR%\*.exp
del %BUILDDIR%\*.ipdb

echo/
copy ..\thirdparty\opencv\x64\vc15\bin\opencv_world342.dll %BUILDDIR%

echo/
set modules=coreLib.dll fp_offlineFrameProvider.dll fp_sisoFGProvLib.dll juneQCSUI.exe statLib.dll logLib.dll algo_fullImageRunner.dll
if exist "%QT_ROOT%\bin\windeployqt.exe" ( 
for %%a in (%modules%) do (
echo "     --- deploying %%a"
"%QT_ROOT%\bin\windeployqt.exe" "%BUILDDIR%\%%a"
))

echo "******************   Build Finished !  ***********************"