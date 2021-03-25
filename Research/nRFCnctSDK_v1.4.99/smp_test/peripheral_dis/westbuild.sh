#!/bin/bash

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin
export PATH=${PATH}:${HOME}/opt/nRF-Command-Line-Tools_10_9_0_OSX/nrfjprog

# bash completion
export NCS_HOME=${HOME}/opt/venv/ncs
source ${NCS_HOME}/west-completion.bash

# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340 DK
    ${NCS_HOME}/bin/west -v flash -d build_signed
else
    # Build for nRF5340 DK
    rm -rfv build_signed
    ${NCS_HOME}/bin/west build -c -b nrf5340dk_nrf5340_cpuapp -d build_signed -- -DCONFIG_MCUBOOT_SIGNATURE_KEY_FILE=\"bootloader/mcuboot/root-rsa-2048.pem\" -DOVERLAY_CONFIG=overlay-smp.conf
fi

deactivate
