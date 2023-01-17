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

set PUBKEYDIR=%4
if "%PUBKEYDIR%" == "" (
    echo "Parameter 4 (public key export directory path) not specified"
    exit 1
)

if not exist "%PUBKEYDIR%" (
    echo "Public key export directory is not exists"
    exit 1
)

set BACKUPDIR=%5
if "%BACKUPDIR%" == "" (
    echo "Parameter 4 (backup directory path) not specified"
    exit 1
)

if not exist "%BACKUPDIR%" (
    echo "Backup directory is not exists"
    exit 1
)

rem Set environment value
set GNUPGHOME=%TEMPDIR%

rem
rem Backup
rem
gpg --armor --pinentry-mode loopback --passphrase %PASSPHRASE% --export-secret-keys %KEYID% > ${TEMPDIR}\master.key
if %ERRORLEVEL% neq 0 (
    echo "Export secret key fail"
    exit 1
)

gpg --armor --pinentry-mode loopback --passphrase %PASSPHRASE% --export-secret-subkeys %KEYID% > ${TEMPDIR}\sub.key
if %ERRORLEVEL% neq 0 (
    echo "Export secret sub key fail"
    exit 1
)

rem
rem Create archive for backup
rem
cd %TEMPDIR%
tar -czf %BACKUPDIR%\GNUPGHOME.tgz .

rem
rem Export public key
rem
gpg --armor --yes --output %PUBKEYDIR%\public_key.pgp --export %KEYID%
if %ERRORLEVEL% neq 0 (
    echo "Export public key fail"
    exit 1
)

rem
rem List all available certificates
rem
gpg -K

echo "Execute script for gnupg success"
exit 0
