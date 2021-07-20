#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
#   nrf52840dk_nrf52840
export BUILD_TARGET=nrf52840dk_nrf52840

# Build target
#   None for Nordic boards
#   MDBT50Q_dongle_rev2
export BOARD_TARGET=MDBT50Q_dongle_rev2

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin
export PATH=${PATH}:${HOME}/opt/nRF-Command-Line-Tools_10_9_0_OSX/nrfjprog
export PATH=${PATH}:.

# bash completion
export REPO_HOME=${HOME}/GitHub/onecard-fido
export NCS_HOME=${REPO_HOME}/pyvenvs/ncs
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
        YML_FILE=configuration/${BUILD_TARGET}/pm_static.yml
        ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build -- -DPM_STATIC_YML_FILE="${YML_FILE}"
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
