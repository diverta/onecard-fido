#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
export BUILD_TARGET=nrf5340dk_nrf5340_cpuapp

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
    if [ "${BUILD_TARGET}" == "nrf5340dk_nrf5340_cpuapp" ]; then
        CONFIG_FILE=boards/nrf5340dk_nrf5340_cpuapp.conf
    else
        CONFIG_FILE=prj.conf
    fi
    grep $1 ${CONFIG_FILE} | sed -e "s/.*\"\(.*\)\"/\1/"
}

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
    # Config for target board
    HW_REV_STR=`retrieve_prj_conf CONFIG_BT_DIS_HW_REV_STR`
    if [ -n "${HW_REV_STR}" ]; then
        DTS_FILE=configuration/${BUILD_TARGET}/${HW_REV_STR}.overlay
        if [ -f ${DTS_FILE} ]; then
            DTS_OPT="-DDTC_OVERLAY_FILE=${DTS_FILE}"
        fi
    fi
    # Build for nRF5340
    rm -rf build_signed
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed -- -DOVERLAY_CONFIG=overlay-smp.conf ${DTS_OPT}
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
    # Deploy binary file for DFU
    FW_REV_STR=`retrieve_prj_conf CONFIG_BT_DIS_FW_REV_STR`
    cp -pv build_signed/zephyr/app_update.bin ../firmwares/secure_device_app/app_update.${HW_REV_STR}.${FW_REV_STR}.bin
    echo Application binary for secure bootloader is now available.
fi

deactivate
exit 0
