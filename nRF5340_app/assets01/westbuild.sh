#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
export BUILD_TARGET=nrf5340dk_nrf5340_cpuapp

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-arm-none-eabi"

# bash completion
export NCS_HOME=${HOME}/opt/ncs_2.2.0
export ZEPHYR_BASE=${NCS_HOME}/zephyr
source ${ZEPHYR_BASE}/zephyr-env.sh
 
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
