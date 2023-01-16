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

# Set gpg command option (additional)
SCRIPT_OPT=$2

# Set environment value
export GNUPGHOME=${TEMPDIR}

# Check if parameter file exist
SCRIPTNAME=`basename $0 .sh`
RESET_COMMAND=${TEMPDIR}/${SCRIPTNAME}.param
if [ ! -f ${RESET_COMMAND} ]; then
    echo "Command parameter file is not exists"
    exit 1
fi

#
# OpenPGP card reset
#
/usr/local/bin/gpg --command-file ${RESET_COMMAND} ${SCRIPT_OPT} --edit-card
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "OpenPGP card reset fail"
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
