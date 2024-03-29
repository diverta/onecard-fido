#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
#   nrf52840dk_nrf52840
export BUILD_TARGET=nrf5340dk_nrf5340_cpuapp

# Build target
#   None for Nordic boards
#   MDBT50Q_dongle_rev2
export BOARD_TARGET=

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin
export PATH=${PATH}:.

# bash completion
export NCS_HOME=${HOME}/opt/ncs_1.9.99
export ZEPHYR_BASE=${NCS_HOME}/zephyr
source ${NCS_HOME}/west-completion.bash

# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340/nRF52840
    ${NCS_HOME}/bin/west -v flash -d build
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    rm -rf build
    if [ "${BUILD_TARGET}" == "nrf5340dk_nrf5340_cpuapp" ]; then
        # Build for nRF5340
        ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build
    else
        # Build for nRF52840
        if [ -n "${BOARD_TARGET}" ]; then
            DTS_FILE=configuration/${BUILD_TARGET}/${BOARD_TARGET}.overlay
            DTS_OPT="-- -DDTC_OVERLAY_FILE=${DTS_FILE}"
        fi
        ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build ${DTS_OPT}
    fi
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
fi

deactivate
exit 0
