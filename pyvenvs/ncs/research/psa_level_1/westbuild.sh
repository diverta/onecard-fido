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
    # Manually flash the MCUboot bootloader image binary
    cd build_signed
    nrfjprog -f NRF53 --program tfm/bin/bl2.hex --sectorerase 
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
    # Flash the concatenated TF-M + Zephyr binary
    ninja flash
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    # Build for nRF5340 DK
    rm -rfv build_signed
    mkdir build_signed
    cd build_signed
    cmake -GNinja -DBOARD=nrf5340dk_nrf5340_cpuappns ..
    ninja
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
fi

deactivate
exit 0
