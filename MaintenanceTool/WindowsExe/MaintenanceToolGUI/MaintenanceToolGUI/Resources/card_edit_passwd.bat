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

rem Check if parameter file exist
set PARAM_FILE_NAME=%2
set CARD_EDIT_COMMAND=%TEMPDIR%\%PARAM_FILE_NAME%
if not exist "%CARD_EDIT_COMMAND%" (
    echo "Command parameter file is not exists"
    exit 1
)

rem Set gpg command option (additional)
set SCRIPT_OPT=%3

rem Set environment value
set GNUPGHOME=%TEMPDIR%

rem
rem OpenPGP card edit passwd/unblock
rem
type %CARD_EDIT_COMMAND% | gpg --command-fd 0 --status-fd 1 --pinentry-mode loopback %SCRIPT_OPT% --edit-card
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card edit passwd/unblock fail"
    exit 1
)

gpg --card-status
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card status fail"
    exit 1
)

gpgconf --kill gpg-agent
if %ERRORLEVEL% neq 0 (
    echo "OpenPGP card kill-agent fail"
    exit 1
)

echo "Execute script for gnupg success"
exit 0
