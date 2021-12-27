#!/usr/bin/env bash

# Check parameters
TEMPDIR=$1
if [ "${TEMPDIR}" == "" ]; then
    echo "Parameter 1 (temp directory path) not specified"
    exit 1
fi
if [ ! -d ${TEMPDIR} ]; then
    echo "Temp directory is not exists"
    exit 1
fi

PASSPHRASE=$2
if [ "${PASSPHRASE}" == "" ]; then
    echo "Parameter 2 (passphrase) not specified"
    exit 1
fi

KEYID=$3
if [ "${KEYID}" == "" ]; then
    echo "Parameter 3 (main key id) not specified"
    exit 1
fi

# Set gpg command option (additional)
SCRIPT_OPT=$4

# Set environment value
export GNUPGHOME=${TEMPDIR}

# Check if parameter file exist
SCRIPTNAME=`basename $0 .sh`
GENKEYS_COMMAND=${TEMPDIR}/${SCRIPTNAME}.param
if [ ! -f ${GENKEYS_COMMAND} ]; then
    echo "Command parameter file is not exists"
    exit 1
fi

# Perform add sub keys command
/usr/local/bin/gpg --command-file ${GENKEYS_COMMAND} --pinentry-mode loopback --passphrase ${PASSPHRASE} ${SCRIPT_OPT} --expert --edit-key ${KEYID}
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Execute script for gnupg fail"
    exit 1
fi

# List all available certificates
/usr/local/bin/gpg -K

echo "Execute script for gnupg success"
exit 0
