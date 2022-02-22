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

# Check if parameter file exist
PARAM_FILE_NAME=$2
CARD_EDIT_COMMAND=${TEMPDIR}/${PARAM_FILE_NAME}
if [ ! -f ${CARD_EDIT_COMMAND} ]; then
    echo "Command parameter file is not exists"
    exit 1
fi

# Set gpg command option (additional)
SCRIPT_OPT=$3

# Set environment value
export GNUPGHOME=${TEMPDIR}

#
# OpenPGP card edit passwd/unblock
#
cat ${CARD_EDIT_COMMAND} | /usr/local/bin/gpg --command-fd 0 --status-fd 1 --pinentry-mode loopback ${SCRIPT_OPT} --edit-card
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "OpenPGP card edit passwd/unblock fail"
    exit 1
fi

/usr/local/bin/gpg --card-status
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "OpenPGP card status fail"
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
