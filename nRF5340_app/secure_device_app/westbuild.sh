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
export PATH=${PATH}:${HOME}/opt/nRF-Command-Line-Tools_10_9_0_OSX/nrfjprog

# bash completion
export REPO_HOME=${HOME}/GitHub/onecard-fido
export NCS_HOME=${REPO_HOME}/pyvenvs/ncs
export ZEPHYR_BASE=${NCS_HOME}/zephyr
source ${NCS_HOME}/west-completion.bash

# Retrieve config value from prj.conf
retrieve_prj_conf() {
    if [ "${BUILD_TARGET}" == "nrf52840dk_nrf52840" ]; then
        CONFIG_FILE=boards/nrf52840dk_nrf52840.conf
    else
        CONFIG_FILE=prj.conf
    fi
    grep $1 ${CONFIG_FILE} | sed -e "s/.*\"\(.*\)\"/\1/"
}

# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340/nRF52840
    ${NCS_HOME}/bin/west -v flash -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    # Build for nRF5340/nRF52840
    rm -rf build_signed
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed -- -DOVERLAY_CONFIG=overlay-smp.conf
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
    # Deploy binary file for DFU
    HW_REV_STR=`retrieve_prj_conf CONFIG_BT_DIS_HW_REV_STR`
    FW_REV_STR=`retrieve_prj_conf CONFIG_BT_DIS_FW_REV_STR`
    cp -pv build_signed/zephyr/app_update.bin ../firmwares/secure_device_app/app_update.${HW_REV_STR}.${FW_REV_STR}.bin
    echo Application binary for secure bootloader is now available.
fi

deactivate
exit 0
