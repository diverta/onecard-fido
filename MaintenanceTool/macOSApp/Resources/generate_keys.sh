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

# Set gpg command option (additional)
SCRIPT_OPT=$3

# Set environment value
export GNUPGHOME=${TEMPDIR}

# Check if parameter file exist
GENKEYS_COMMAND=${TEMPDIR}/generate_keys.param
if [ ! -f ${GENKEYS_COMMAND} ]; then
    echo "Command file for generating keys is not exists"
    exit 1
fi

# Perform generate keys command
/usr/local/bin/gpg --command-file ${GENKEYS_COMMAND} --pinentry-mode loopback --passphrase ${PASSPHRASE} --expert --full-gen-key ${SCRIPT_OPT}
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Execute script for gnupg fail"
    exit 1
fi

echo "Execute script for gnupg success"
exit 0
