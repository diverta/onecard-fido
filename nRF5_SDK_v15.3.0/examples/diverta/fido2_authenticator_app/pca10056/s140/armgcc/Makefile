PROJECT_NAME     := fido2_authenticator_app_pca10056_s140
TARGETS          := nrf52840_xxaa
OUTPUT_DIRECTORY := _build
TARGET_BOARD     := PCA10059

# application version info
FW_REV := 0.2.8
CFLAGS += -DFW_REV=\"$(FW_REV)\"
# hardware version info
HW_REV := 0.2
CFLAGS += -DHW_REV=\"$(HW_REV)\"

SDK_ROOT := $(HOME)/opt/nRF5_SDK_15.3.0
PROJ_DIR := ../../..
FIDO_DIR := $(PROJ_DIR)/../../../../FIDO2Device
CCID_DIR := $(PROJ_DIR)/../../../../CCID
FD2LIB_DIR := $(FIDO_DIR)/fido2_lib
U2FLIB_DIR := $(FIDO_DIR)/u2f_lib
CT2LIB_DIR := $(FIDO_DIR)/ctap2_lib
CIDLIB_DIR := $(CCID_DIR)/ccid_lib
PLTLIB_DIR := $(PROJ_DIR)/../plat_lib
BLELIB_DIR := $(PROJ_DIR)/../ble_lib
DMOLIB_DIR := $(PROJ_DIR)/../demo_lib
SDK_CUSTOM_ROOT := $(PROJ_DIR)/../../..
DEPLOY_ROOT := $(SDK_CUSTOM_ROOT)/firmwares

# tinycbor root directory
TINYCBOR_ROOT := $(HOME)/GitHub/tinycbor

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ble/ble_db_discovery/ble_db_discovery.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_dis/ble_dis.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus_c/ble_nus_c.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_params.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_state.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
  $(SDK_ROOT)/components/ble/nrf_ble_scan/nrf_ble_scan.c \
  $(SDK_ROOT)/components/ble/peer_manager/auth_status_tracker.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/nrf_ble_lesc.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager_handler.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/atomic_flags/nrf_atflags.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp_btn_ble.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp.c \
  $(SDK_ROOT)/components/libraries/button/app_button.c \
  $(SDK_ROOT)/components/libraries/crc16/crc16.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_ecc.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_ecdh.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_ecdsa.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_hash.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_hmac.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_init.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_mutex.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_rng.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310/cc310_backend_shared.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/mbedtls/mbedtls_backend_aes.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/mbedtls/mbedtls_backend_init.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw/nrf_hw_backend_init.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw/nrf_hw_backend_rng_mbedtls.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw/nrf_hw_backend_rng.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_aead.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_aes_shared.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_aes.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_ecc.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_ecdh.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_ecdsa.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_eddsa.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_error.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_hash.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_hkdf.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_hmac.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_init.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_rng.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_shared.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/fds/fds.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
  $(SDK_ROOT)/components/libraries/hardfault/nrf52/handler/hardfault_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_rtt.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_uart.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/components/libraries/mem_manager/mem_manager.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c \
  $(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_core.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_string_desc.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd.c \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/app_usbd_hid.c \
  $(SDK_CUSTOM_ROOT)/components/libraries/usbd/class/hid/generic/app_usbd_hid_generic.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/nfc/platform/nfc_platform.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/external/fnmatch/fnmatch.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/mbedtls/library/aes.c \
  $(SDK_ROOT)/external/mbedtls/library/ctr_drbg.c \
  $(SDK_ROOT)/external/utf_converter/utf.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_power.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_rng.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_twi.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_nfct.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_power.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rng.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_systick.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_timer.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twi.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twim.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_usbd.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/external/mbedtls/library/aes.c \
  $(SDK_ROOT)/external/mbedtls/library/aesni.c \
  $(SDK_ROOT)/external/mbedtls/library/bignum.c \
  $(SDK_ROOT)/external/mbedtls/library/cipher.c \
  $(SDK_ROOT)/external/mbedtls/library/cipher_wrap.c \
  $(SDK_ROOT)/external/mbedtls/library/des.c \
  $(SDK_ROOT)/external/mbedtls/library/ecp.c \
  $(SDK_ROOT)/external/mbedtls/library/ecp_curves.c \
  $(SDK_ROOT)/external/mbedtls/library/memory_buffer_alloc.c \
  $(SDK_ROOT)/external/mbedtls/library/platform.c \
  $(PROJ_DIR)/main.c \
  $(FD2LIB_DIR)/fido_ble_receive.c \
  $(FD2LIB_DIR)/fido_ble_send.c \
  $(FD2LIB_DIR)/fido_command.c \
  $(FD2LIB_DIR)/fido_common.c \
  $(FD2LIB_DIR)/fido_ctap2_command.c \
  $(FD2LIB_DIR)/fido_hid_channel.c \
  $(FD2LIB_DIR)/fido_hid_receive.c \
  $(FD2LIB_DIR)/fido_hid_send.c \
  $(FD2LIB_DIR)/fido_maintenance.c \
  $(FD2LIB_DIR)/fido_maintenance_cryption.c \
  $(FD2LIB_DIR)/fido_nfc_receive.c \
  $(FD2LIB_DIR)/fido_nfc_send.c \
  $(FD2LIB_DIR)/fido_receive_apdu.c \
  $(FD2LIB_DIR)/fido_u2f_command.c \
  $(CT2LIB_DIR)/ctap2_cbor_authgetinfo.c \
  $(CT2LIB_DIR)/ctap2_cbor_encode.c \
  $(CT2LIB_DIR)/ctap2_cbor_parse.c \
  $(CT2LIB_DIR)/ctap2_client_pin.c \
  $(CT2LIB_DIR)/ctap2_client_pin_token.c \
  $(CT2LIB_DIR)/ctap2_common.c \
  $(CT2LIB_DIR)/ctap2_extension_hmac_secret.c \
  $(CT2LIB_DIR)/ctap2_get_assertion.c \
  $(CT2LIB_DIR)/ctap2_make_credential.c \
  $(CT2LIB_DIR)/ctap2_pubkey_credential.c \
  $(CIDLIB_DIR)/ccid_apdu.c \
  $(CIDLIB_DIR)/ccid_main.c \
  $(CIDLIB_DIR)/ccid_piv.c \
  $(CIDLIB_DIR)/ccid_piv_general_auth.c \
  $(CIDLIB_DIR)/ccid_piv_object.c \
  $(PLTLIB_DIR)/fido_board.c \
  $(PLTLIB_DIR)/fido_crypto.c \
  $(PLTLIB_DIR)/fido_crypto_aes_cbc_256.c \
  $(PLTLIB_DIR)/fido_crypto_keypair.c \
  $(PLTLIB_DIR)/fido_crypto_sskey.c \
  $(PLTLIB_DIR)/fido_flash_blp_auth_param.c \
  $(PLTLIB_DIR)/fido_flash_client_pin_store.c \
  $(PLTLIB_DIR)/fido_flash_common.c \
  $(PLTLIB_DIR)/fido_flash_pairing_mode.c \
  $(PLTLIB_DIR)/fido_flash_token_counter.c \
  $(PLTLIB_DIR)/fido_status_indicator.c \
  $(PLTLIB_DIR)/fido_timer.c \
  $(PLTLIB_DIR)/fido_twi.c \
  $(PLTLIB_DIR)/nfc_service.c \
  $(PLTLIB_DIR)/usbd_service.c \
  $(PLTLIB_DIR)/usbd_service_ccid.c \
  $(PLTLIB_DIR)/usbd_service_cdc.c \
  $(U2FLIB_DIR)/u2f_authenticate.c \
  $(U2FLIB_DIR)/u2f_keyhandle.c \
  $(U2FLIB_DIR)/u2f_register.c \
  $(U2FLIB_DIR)/u2f_signature.c \
  $(BLELIB_DIR)/ble_service_central.c \
  $(BLELIB_DIR)/ble_service_central_stat.c \
  $(BLELIB_DIR)/ble_service_common.c \
  $(BLELIB_DIR)/ble_service_peripheral.c \
  $(BLELIB_DIR)/fido_ble_event.c \
  $(BLELIB_DIR)/fido_ble_pairing.c \
  $(BLELIB_DIR)/fido_ble_send_retry.c \
  $(BLELIB_DIR)/fido_ble_service.c \
  $(DMOLIB_DIR)/demo_cdc_service.c \
  $(DMOLIB_DIR)/demo_ble_peripheral_auth.c \
  $(PLTLIB_DIR)/app_usbd_ccid.c \

# source control for former hardware version
ifeq ("$(HW_REV)","0.2")
  FD2LIB_REV2_DIR := $(FIDO_DIR)/fido2_lib_rev2
  PLTLIB_REV2_DIR := $(PROJ_DIR)/../plat_lib_rev2
  SRC_FILES += \
  $(FD2LIB_REV2_DIR)/fido_command_common.c \
  $(FD2LIB_REV2_DIR)/fido_maintenance_skcert.c \
  $(PLTLIB_REV2_DIR)/fido_flash_event.c \
  $(PLTLIB_REV2_DIR)/fido_flash_password.c \
  $(PLTLIB_REV2_DIR)/fido_flash_skey_cert.c \

else
  FD2LIB_REV2_DIR := $(FIDO_DIR)/fido2_lib_rev2_1
  PLTLIB_REV2_DIR := $(PROJ_DIR)/../plat_lib_rev2_1
  FCALIB_DIR := $(FIDO_DIR)/fido_cryptoauth_lib
  CRPLIB_DIR := $(HOME)/GitHub/cryptoauthlib/lib
  SRC_FILES += \
  $(FD2LIB_REV2_DIR)/fido_command_common.c \
  $(FD2LIB_REV2_DIR)/fido_maintenance_skcert.c \
  $(PLTLIB_REV2_DIR)/fido_flash_event.c \
  $(PLTLIB_REV2_DIR)/fido_flash_skey_cert.c \
  $(FCALIB_DIR)/fido_cryptoauth.c \
  $(FCALIB_DIR)/fido_cryptoauth_aes_cbc.c \
  $(FCALIB_DIR)/fido_cryptoauth_setup.c \
  $(CRPLIB_DIR)/atca_cfgs.c \
  $(CRPLIB_DIR)/atca_command.c \
  $(CRPLIB_DIR)/atca_device.c \
  $(CRPLIB_DIR)/atca_execution.c \
  $(CRPLIB_DIR)/atca_iface.c \
  $(CRPLIB_DIR)/basic/atca_basic.c \
  $(CRPLIB_DIR)/basic/atca_basic_aes.c \
  $(CRPLIB_DIR)/basic/atca_basic_aes_cbc.c \
  $(CRPLIB_DIR)/basic/atca_basic_ecdh.c \
  $(CRPLIB_DIR)/basic/atca_basic_gendig.c \
  $(CRPLIB_DIR)/basic/atca_basic_genkey.c \
  $(CRPLIB_DIR)/basic/atca_basic_info.c \
  $(CRPLIB_DIR)/basic/atca_basic_lock.c \
  $(CRPLIB_DIR)/basic/atca_basic_nonce.c \
  $(CRPLIB_DIR)/basic/atca_basic_privwrite.c \
  $(CRPLIB_DIR)/basic/atca_basic_random.c \
  $(CRPLIB_DIR)/basic/atca_basic_read.c \
  $(CRPLIB_DIR)/basic/atca_basic_sha.c \
  $(CRPLIB_DIR)/basic/atca_basic_sign.c \
  $(CRPLIB_DIR)/basic/atca_basic_updateextra.c \
  $(CRPLIB_DIR)/basic/atca_basic_verify.c \
  $(CRPLIB_DIR)/basic/atca_basic_write.c \
  $(CRPLIB_DIR)/basic/atca_helpers.c \
  $(CRPLIB_DIR)/crypto/atca_crypto_sw_sha2.c \
  $(CRPLIB_DIR)/crypto/hashes/sha2_routines.c \
  $(CRPLIB_DIR)/hal/atca_hal.c \
  $(CRPLIB_DIR)/host/atca_host.c \
  $(PLTLIB_DIR)/atecc608a_i2c_hal.c \

endif

# Include folders common to all targets
INC_FOLDERS += \
  ../config \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/ble/ble_db_discovery \
  $(SDK_ROOT)/components/ble/ble_dtm \
  $(SDK_ROOT)/components/ble/ble_racp \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_cscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_cts_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_dfu \
  $(SDK_ROOT)/components/ble/ble_services/ble_dis \
  $(SDK_ROOT)/components/ble/ble_services/ble_gls \
  $(SDK_ROOT)/components/ble/ble_services/ble_hids \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_hts \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lls \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_tps \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr \
  $(SDK_ROOT)/components/ble/nrf_ble_scan \
  $(SDK_ROOT)/components/ble/peer_manager \
  $(SDK_CUSTOM_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/libraries/atomic_flags \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/bootloader \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/button \
  $(SDK_ROOT)/components/libraries/cli \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/crypto \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310 \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310_bl \
  $(SDK_ROOT)/components/libraries/crypto/backend/cifra \
  $(SDK_ROOT)/components/libraries/crypto/backend/mbedtls \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_sw \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon \
  $(SDK_ROOT)/components/libraries/crypto/backend/optiga \
  $(SDK_ROOT)/components/libraries/csense \
  $(SDK_ROOT)/components/libraries/csense_drv \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/experimental_task_manager \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/gfx \
  $(SDK_ROOT)/components/libraries/gpiote \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/hci \
  $(SDK_ROOT)/components/libraries/led_softblink \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/low_power_pwm \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/mpu \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/pwm \
  $(SDK_ROOT)/components/libraries/pwr_mgmt \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/sdcard \
  $(SDK_ROOT)/components/libraries/slip \
  $(SDK_ROOT)/components/libraries/sortlist \
  $(SDK_ROOT)/components/libraries/spi_mngr \
  $(SDK_ROOT)/components/libraries/stack_guard \
  $(SDK_ROOT)/components/libraries/stack_info \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/svc \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/twi_mngr \
  $(SDK_ROOT)/components/libraries/twi_sensor \
  $(SDK_ROOT)/components/libraries/usbd \
  $(SDK_ROOT)/components/libraries/usbd/class/audio \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
  $(SDK_ROOT)/components/libraries/usbd/class/hid \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/generic \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/kbd \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/mouse \
  $(SDK_ROOT)/components/libraries/usbd/class/msc \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ac_rec_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ble_oob_advdata_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/le_oob_rec_parser \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ac_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_oob_advdata \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_lib \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_msg \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/common \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ep_oob_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/hs_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/le_oob_rec \
  $(SDK_ROOT)/components/nfc/ndef/generic/message \
  $(SDK_ROOT)/components/nfc/ndef/generic/record \
  $(SDK_ROOT)/components/nfc/ndef/launchapp \
  $(SDK_ROOT)/components/nfc/ndef/parser/message \
  $(SDK_ROOT)/components/nfc/ndef/parser/record \
  $(SDK_ROOT)/components/nfc/ndef/text \
  $(SDK_ROOT)/components/nfc/ndef/uri \
  $(SDK_ROOT)/components/nfc/t2t_lib \
  $(SDK_ROOT)/components/nfc/t2t_parser \
  $(SDK_ROOT)/components/nfc/t4t_lib \
  $(SDK_ROOT)/components/nfc/t4t_parser/apdu \
  $(SDK_ROOT)/components/nfc/t4t_parser/cc_file \
  $(SDK_ROOT)/components/nfc/t4t_parser/hl_detection_procedure \
  $(SDK_ROOT)/components/nfc/t4t_parser/tlv \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/softdevice/s140/headers \
  $(SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/external/fnmatch \
  $(SDK_ROOT)/external/fprintf \
  $(SDK_ROOT)/external/mbedtls/include \
  $(SDK_ROOT)/external/nrf_cc310/include \
  $(SDK_ROOT)/external/nrf_oberon \
  $(SDK_ROOT)/external/nrf_oberon/include \
  $(SDK_ROOT)/external/nrf_tls/mbedtls/nrf_crypto/config \
  $(SDK_ROOT)/external/segger_rtt \
  $(SDK_ROOT)/external/utf_converter \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/soc \
  $(PROJ_DIR) \
  $(PLTLIB_DIR) \
  $(FD2LIB_DIR) \
  $(U2FLIB_DIR) \
  $(CT2LIB_DIR) \
  $(CIDLIB_DIR) \
  $(FCALIB_DIR) \
  $(BLELIB_DIR) \
  $(DMOLIB_DIR) \
  $(TINYCBOR_ROOT)/src \

# source control for former hardware version
ifeq ("$(HW_REV)","0.2")
  INC_FOLDERS += \
  $(FD2LIB_REV2_DIR) \
  $(PLTLIB_REV2_DIR) \

else
  INC_FOLDERS += \
  $(FD2LIB_REV2_DIR) \
  $(PLTLIB_REV2_DIR) \
  $(CRPLIB_DIR) \

endif

# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.12.a \
  $(TINYCBOR_ROOT)/lib/libtinycbor.a \
  $(SDK_ROOT)/components/nfc/t4t_lib/nfc_t4t_lib_gcc.a \

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DBOARD_$(TARGET_BOARD)
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DMBEDTLS_CONFIG_FILE=\"nrf_crypto_mbedtls_config.h\"
CFLAGS += -DMBEDTLS_DES_C
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DNRF_CRYPTO_MAX_INSTANCE_COUNT=1
CFLAGS += -DNRF_SD_BLE_API_VERSION=6
CFLAGS += -DS140
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DSWI_DISABLE0
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# for CRYPTOAUTH library
CFLAGS += -DATCA_HAL_I2C -DATCA_NO_POLL -Wno-unused-variable

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DBOARD_$(TARGET_BOARD)
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -DNRF_CRYPTO_MAX_INSTANCE_COUNT=1
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=6
ASMFLAGS += -DS140
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DSWI_DISABLE0

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52840_xxaa: CFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.PHONY: default help deploy_dk deploy secure_pkg

# Default target - first one defined
ifeq ("$(TARGET_BOARD)","PCA10056")
    default: $(TARGETS) deploy_dk
else
    default: $(TARGETS) deploy secure_pkg
endif

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52840_xxaa
	@echo		flash_softdevice
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

deploy_dk:
	/bin/cp -pv $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex $(DEPLOY_ROOT)/nrf52840_DK_xxaa.hex
	@echo Application hex file for NRF52840 DK is now available.

deploy:
	/bin/cp -pv $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex $(DEPLOY_ROOT)/nrf52840_xxaa.hex
	@echo Application hex file for MDBT50Q Dongle is now available.

secure_pkg:
	/usr/local/bin/nrfutil pkg generate --hw-version 52 --sd-req 0xB6 --application-version-string $(FW_REV) --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --key-file $(PROJ_DIR)/../../dfu/private.pem $(DEPLOY_ROOT)/app_dfu_package.$(FW_REV).zip
	@echo Application zip package for secure bootloader is now available.

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash flash_softdevice erase

# Flash the program
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --sectorerase
	nrfjprog -f nrf52 --reset

# Flash softdevice
flash_softdevice:
	@echo Flashing: s140_nrf52_6.1.1_softdevice.hex
	nrfjprog -f nrf52 --program $(SDK_ROOT)/components/softdevice/s140/hex/s140_nrf52_6.1.1_softdevice.hex --sectorerase
	nrfjprog -f nrf52 --reset

erase:
	nrfjprog -f nrf52 --eraseall

SDK_CONFIG_FILE := ../config/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
