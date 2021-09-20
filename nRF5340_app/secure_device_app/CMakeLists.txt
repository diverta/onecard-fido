# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(secure_device_app)

if(CONFIG_MCUMGR_SMP_BT_CUSTOM)
    add_subdirectory(zephyr_custom/subsys/mgmt/mcumgr)
endif()

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
    ${app_sources}
    ../../FIDO2Device/ctap2_lib/ctap2_cbor_encode.c
    ../../FIDO2Device/ctap2_lib/ctap2_cbor_parse.c
    ../../FIDO2Device/ctap2_lib/ctap2_client_pin.c
    ../../FIDO2Device/ctap2_lib/ctap2_client_pin_token.c
    ../../FIDO2Device/fido2_lib/fido_ble_receive.c
    ../../FIDO2Device/fido2_lib/fido_ble_send.c
    ../../FIDO2Device/fido2_lib/fido_command.c
    ../../FIDO2Device/fido2_lib/fido_command_common.c
    ../../FIDO2Device/fido2_lib/fido_common.c
    ../../FIDO2Device/fido2_lib/fido_crypto.c
    ../../FIDO2Device/fido2_lib/fido_ctap2_command.c
    ../../FIDO2Device/fido2_lib/fido_hid_channel.c
    ../../FIDO2Device/fido2_lib/fido_hid_receive.c
    ../../FIDO2Device/fido2_lib/fido_hid_send.c
    ../../FIDO2Device/fido2_lib/fido_maintenance.c
    ../../FIDO2Device/fido2_lib/fido_maintenance_cryption.c
    ../../FIDO2Device/fido2_lib/fido_maintenance_skcert.c
    ../../FIDO2Device/fido2_lib/fido_receive_apdu.c
    ../../FIDO2Device/fido2_lib/fido_u2f_command.c
    ../../../tinycbor/src/cborencoder.c
    ../../../tinycbor/src/cborerrorstrings.c
    ../../../tinycbor/src/cborparser.c
)
target_include_directories(app PRIVATE 
    include
    ../../FIDO2Device/ctap2_lib
    ../../FIDO2Device/fido2_lib
    ../../FIDO2Device/u2f_lib
    ../../../tinycbor/src
)
