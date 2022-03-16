#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
export BUILD_TARGET=nrf5340dk_nrf5340_cpuapp

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin

# bash completion
export NCS_HOME=${HOME}/opt/ncs_1.9.1
export ZEPHYR_BASE=${NCS_HOME}/zephyr
source ${NCS_HOME}/west-completion.bash
 
# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340
    ${NCS_HOME}/bin/west -v flash -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    # Build for nRF5340
    rm -rf build_signed
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
fi

deactivate
exit 0
