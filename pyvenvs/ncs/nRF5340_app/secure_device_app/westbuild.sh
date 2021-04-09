#!/bin/bash

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin
export PATH=${PATH}:${HOME}/opt/nRF-Command-Line-Tools_10_9_0_OSX/nrfjprog

# bash completion
export REPO_HOME=${HOME}/GitHub/onecard-fido
export NCS_HOME=${REPO_HOME}/pyvenvs/ncs
source ${NCS_HOME}/west-completion.bash

# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340 DK
    ${NCS_HOME}/bin/west -v flash -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    # Build for nRF5340 DK
    rm -rfv build_signed
    ${NCS_HOME}/bin/west build -c -b nrf5340dk_nrf5340_cpuapp -d build_signed -- -DCONFIG_MCUBOOT_SIGNATURE_KEY_FILE=\"bootloader/mcuboot/root-rsa-2048.pem\" -DOVERLAY_CONFIG=overlay-smp.conf
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
    # Deploy binary file for DFU
    cp -pv build_signed/zephyr/app_update.bin ${REPO_HOME}/nRF5340_app/firmwares/secure_device_app/
    echo Application binary for secure bootloader is now available.
fi

deactivate
exit 0
