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

PUBKEYDIR=$4
if [ "${PUBKEYDIR}" == "" ]; then
    echo "Parameter 4 (public key export directory path) not specified"
    exit 1
fi
if [ ! -d ${PUBKEYDIR} ]; then
    echo "Public key export directory is not exists"
    exit 1
fi

BACKUPDIR=$5
if [ "${BACKUPDIR}" == "" ]; then
    echo "Parameter 4 (backup directory path) not specified"
    exit 1
fi
if [ ! -d ${BACKUPDIR} ]; then
    echo "Backup directory is not exists"
    exit 1
fi

# Set environment value
export GNUPGHOME=${TEMPDIR}

#
# Backup
#
/usr/local/bin/gpg --armor --pinentry-mode loopback --passphrase ${PASSPHRASE} --export-secret-keys ${KEYID} > ${TEMPDIR}/master.key
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Export secret key fail"
    exit 1
fi

/usr/local/bin/gpg --armor --pinentry-mode loopback --passphrase ${PASSPHRASE} --export-secret-subkeys ${KEYID} > ${TEMPDIR}/sub.key
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Export secret sub key fail"
    exit 1
fi

# Create archive for backup
cd ${TEMPDIR}
/usr/bin/tar -czf GNUPGHOME.tgz . > /dev/null 2>&1
/bin/mv GNUPGHOME.tgz ${BACKUPDIR}/

#
# Export public key
#
/usr/local/bin/gpg --armor --yes --output ${PUBKEYDIR}/public_key.pgp --export ${KEYID}
RC=`echo $?`
if [ ${RC} -ne 0 ]; then
    echo "Export public key fail"
    exit 1
fi

# List all available certificates
/usr/local/bin/gpg -K

echo "Execute script for gnupg success"
exit 0
