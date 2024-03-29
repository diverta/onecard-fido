# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(secure_device_app)

FILE(GLOB app_sources src/*.c)
FILE(GLOB wrapper_sources wrapper/*.c)
target_sources(app PRIVATE
    ${app_sources}
    ${wrapper_sources}
    $ENV{NCS_HOME}/mbedtls/library/ctr_drbg.c
    ../../FIDO2Device/ctap2_lib/ctap2_cbor.c
    ../../FIDO2Device/ctap2_lib/ctap2_cbor_authgetinfo.c
    ../../FIDO2Device/ctap2_lib/ctap2_cbor_encode.c
    ../../FIDO2Device/ctap2_lib/ctap2_cbor_parse.c
    ../../FIDO2Device/ctap2_lib/ctap2_client_pin.c
    ../../FIDO2Device/ctap2_lib/ctap2_client_pin_token.c
    ../../FIDO2Device/ctap2_lib/ctap2_common.c
    ../../FIDO2Device/ctap2_lib/ctap2_extension_hmac_secret.c
    ../../FIDO2Device/ctap2_lib/ctap2_get_assertion.c
    ../../FIDO2Device/ctap2_lib/ctap2_make_credential.c
    ../../FIDO2Device/ctap2_lib/ctap2_pubkey_credential.c
    ../../FIDO2Device/fido2_lib/fido_ble_receive.c
    ../../FIDO2Device/fido2_lib/fido_ble_send.c
    ../../FIDO2Device/fido2_lib/fido_command.c
    ../../FIDO2Device/fido2_lib/fido_command_common.c
    ../../FIDO2Device/fido2_lib/fido_common.c
    ../../FIDO2Device/fido2_lib/fido_ctap2_command.c
    ../../FIDO2Device/fido2_lib/fido_hid_channel.c
    ../../FIDO2Device/fido2_lib/fido_hid_receive.c
    ../../FIDO2Device/fido2_lib/fido_hid_send.c
    ../../FIDO2Device/fido2_lib/fido_maintenance.c
    ../../FIDO2Device/fido2_lib/fido_maintenance_attestation.c
    ../../FIDO2Device/fido2_lib/fido_receive_apdu.c
    ../../FIDO2Device/fido2_lib/fido_u2f_command.c
    ../../FIDO2Device/u2f_lib/u2f_authenticate.c
    ../../FIDO2Device/u2f_lib/u2f_keyhandle.c
    ../../FIDO2Device/u2f_lib/u2f_register.c
    ../../FIDO2Device/u2f_lib/u2f_signature.c
    ../../CCID/ccid_lib/ccid_apdu.c
    ../../CCID/ccid_lib/ccid_main.c
    ../../CCID/ccid_lib/ccid_process.c
    ../../CCID/piv_lib/ccid_piv.c
    ../../CCID/piv_lib/ccid_piv_authenticate.c
    ../../CCID/piv_lib/ccid_piv_general_auth.c
    ../../CCID/piv_lib/ccid_piv_object.c
    ../../CCID/piv_lib/ccid_piv_object_import.c
    ../../CCID/piv_lib/ccid_piv_pin.c
    ../../CCID/piv_lib/ccid_piv_pin_auth.c
    ../../CCID/piv_lib/ccid_piv_pin_update.c
    ../../CCID/piv_lib/ccid_ykpiv.c
    ../../CCID/piv_lib/ccid_ykpiv_import_key.c
    ../../CCID/openpgp_lib/ccid_openpgp.c
    ../../CCID/openpgp_lib/ccid_openpgp_attr.c
    ../../CCID/openpgp_lib/ccid_openpgp_crypto.c
    ../../CCID/openpgp_lib/ccid_openpgp_data.c
    ../../CCID/openpgp_lib/ccid_openpgp_key.c
    ../../CCID/openpgp_lib/ccid_openpgp_key_rsa.c
    ../../CCID/openpgp_lib/ccid_openpgp_object.c
    ../../CCID/openpgp_lib/ccid_openpgp_pin.c
    ../../CCID/openpgp_lib/ccid_pin_auth.c
    ../../CCID/oath_lib/ccid_oath.c
    ../../CCID/oath_lib/ccid_oath_account.c
    ../../CCID/oath_lib/ccid_oath_calculate.c
    ../../CCID/oath_lib/ccid_oath_list.c
    ../../CCID/oath_lib/ccid_oath_object.c
    ../../CCID/oath_lib/ccid_oath_totp.c
    ../../TFT/tiny_tft_lib/tiny_tft.c
    ../../TFT/tiny_tft_lib/tiny_tft_const.c
)
target_include_directories(app PRIVATE 
    $ENV{NCS_HOME}/modules/lib/tinycbor/include
    $ENV{ZEPHYR_BASE}/subsys/settings/include
    include
    ../../FIDO2Device/ctap2_lib
    ../../FIDO2Device/fido2_lib
    ../../FIDO2Device/u2f_lib
    ../../FIDO2Device/wrapper_header
    ../../CCID/ccid_lib
    ../../CCID/oath_lib
    ../../CCID/openpgp_lib
    ../../CCID/piv_lib
    ../../TFT/tiny_tft_lib
)
