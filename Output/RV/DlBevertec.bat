@echo off
rem ******************************************************************
rem WARNING : PLEASE DO NOT ALTER THE CONTENTS OF THIS BATCH FILE
rem ******************************************************************
rem This batch file is used to download the VMAC sample DMLAB1 
rem This sample requires ACT shared library in Gid15 of Flash
rem ******************************************************************
cls
if "%1" == "" goto INVALIDPARAMS
if "%2" == "" goto INVALIDPARAMS
if "%3" == "" goto INVALIDPARAMS

if "%1" == "R" goto release
if "%1" == "D" goto debug
if "%1" == "r" goto release
if "%1" == "d" goto debug
goto exit

:release
if "%2" == "" goto INVALIDPARAMS
if "%2" == "R" goto ramRel
if "%2" == "F" goto flashRel
if "%2" == "r" goto ramRel
if "%2" == "f" goto flashRel
goto exit

:debug
if "%2" == "" goto INVALIDPARAMS
if "%2" == "R" goto ramDbg
if "%2" == "F" goto flashDbg
if "%2" == "r" goto ramDbg
if "%2" == "f" goto flashDbg
goto exit

:ramRel
echo Downloading release version the VMAC Sample components to Ram 
%VRXSDK%\bin\ddl -b115200 setgroup."%3" -FDMLab1RelR.dld
echo Downloading Sample  Complete.
goto exit

:flashRel
echo Downloading release version the VMAC Sample components to Flash 
%VRXSDK%\bin\ddl -p9 -b115200 setgroup."%3" -FBevertecRelF.dld
echo Downloading Sample  Complete.
goto exit

:ramDbg
echo Downloading Debug version the VMAC Sample components to Ram 
%VRXSDK%\bin\ddl -b115200 setgroup."%3" -FBevertecDbgR.dld
echo Downloading Sample  Complete.
goto exit

:flashDbg
echo Downloading Debug version the VMAC Sample components to Flash 
%VRXSDK%\bin\ddl -p9 -b115200 setgroup."%3" -FBevertecDbgf.dld
echo Downloading Sample  Complete.
goto exit

:INVALIDPARAMS
echo -------------------------------------------------------------------------------
echo USAGE : DlBevertec [R/r] [R/r] [g] for Release version to gid g of Ram 
echo       : DlBevertec [R/r] [F/f] [g] for Release version to gid g of Flash 
echo       : DlBevertec [D/d] [R/r] [g] for Debug version to gid g of Ram 
echo       : DlBevertec [D/d] [F/f] [g] for Debug version to gid g of Flash 
echo --------------------------------------------------------------------------------
:exit
