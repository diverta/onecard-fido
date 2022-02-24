#!/usr/bin/env bash

#
# Card status command
#
/usr/local/bin/gpg --card-status
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Card status command fail"
    exit 1
fi

/usr/local/MacGPG2/bin/gpgconf --kill gpg-agent
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "OpenPGP card kill-agent fail"
    exit 1
fi

echo "Execute script for gnupg success"
exit 0
