@echo off

rem Check parameters
set TEMPDIR=%1
if "%TEMPDIR%" == "" (
    echo "Parameter 1 (temp directory path) not specified"
    exit 1
)

if not exist "%TEMPDIR%" (
    echo "Temp directory is not exists"
    exit 1
)

rem Set gpg command option (additional)
set SCRIPT_OPT=%2

rem Set environment value
set GNUPGHOME=%TEMPDIR%

rem Check if parameter file exist
set SCRIPTNAME=%~n0
set RESET_COMMAND=%TEMPDIR%\%SCRIPTNAME%.param
if not exist "%RESET_COMMAND%" (
    echo "Command parameter file is not exists"
    exit 1
)

rem
rem OpenPGP card reset
rem
gpg --command-file %RESET_COMMAND% %SCRIPT_OPT% --edit-card
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card reset fail"
    exit 1
)

gpg --card-status
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card status fail"
    exit 1
)

echo "Execute script for gnupg success"
exit 0
