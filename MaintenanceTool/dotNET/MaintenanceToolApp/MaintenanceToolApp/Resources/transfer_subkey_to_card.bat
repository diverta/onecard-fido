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

set PASSPHRASE=%2
if "%PASSPHRASE%" == "" (
    echo "Parameter 2 (passphrase) not specified"
    exit 1
)

set KEYID=%3
if "%KEYID%" == "" (
    echo "Parameter 3 (main key id) not specified"
    exit 1
)

rem Set gpg command option (additional)
set SCRIPT_OPT=%4

rem Set environment value
set GNUPGHOME=%TEMPDIR%

rem Check if parameter file exist
set SCRIPTNAME=%~n0
set GENKEYS_COMMAND=%TEMPDIR%\%SCRIPTNAME%.param
if not exist "%GENKEYS_COMMAND%" (
    echo "Command parameter file is not exists"
    exit 1
)

rem
rem Perform transfer sub keys command
rem
type %GENKEYS_COMMAND% | gpg --command-fd 0 --pinentry-mode loopback --passphrase %PASSPHRASE% %SCRIPT_OPT% --edit-key %KEYID%
if %ERRORLEVEL% neq 0 (
    echo "Execute script for gnupg fail"
    exit 1
)

rem
rem List all available certificates
rem
gpg -K
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP list cert fail"
    exit 1
)

gpgconf --kill gpg-agent
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card kill-agent fail"
    exit 1
)

echo "Execute script for gnupg success"
exit 0
