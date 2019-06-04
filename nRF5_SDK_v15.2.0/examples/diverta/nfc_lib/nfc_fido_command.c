/* 
 * File:   nfc_fido_command.c
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#include "sdk_common.h"
#include "fds.h"

#include "fido_ctap2_command.h"
#include "nfc_fido_receive.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

void nfc_fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // Flash ROM更新完了時の処理を実行
    uint8_t cmd = nfc_fido_receive_apdu()->INS;
    switch (cmd) {
        case 0x10:
            // NFC CTAP2 command
            fido_ctap2_command_cbor_send_response(p_evt);
            break;
        default:
            break;
    }
}
