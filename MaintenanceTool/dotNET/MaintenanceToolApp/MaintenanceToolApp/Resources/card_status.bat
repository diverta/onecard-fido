@echo off

rem
rem OpenPGP card status
rem
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

exit 0
