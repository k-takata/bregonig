@echo off
if "%1"=="" goto usage
setlocal DISABLEDELAYEDEXPANSION

:: 7z (official version) or 7-zip32 (undll + common archiver) can be used
if "%SEVENZIP%"=="" set SEVENZIP=7-zip32

if not exist x64 mkdir x64

cd src
copy /y objx86\*.lib .
"%SEVENZIP%" a -m0=PPMd ..\src.7z @..\srcfiles.lst
del *.lib
cd ..

copy /y src\objx86\*.dll .
copy /y src\objx64\bregonig.dll x64
"%SEVENZIP%" a -mx=9 %1 @pkgfiles.lst
del *.dll x64\*.dll src.7z
rd x64

goto end

:usage
echo.
echo usage: pack ^<filename^>
echo.
echo ^<filename^>: ex. bron400.zip

:end
