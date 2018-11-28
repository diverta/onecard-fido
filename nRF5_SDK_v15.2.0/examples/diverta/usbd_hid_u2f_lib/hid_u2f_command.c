#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fds.h"

#include "usbd_hid_u2f.h"
#include "hid_u2f_common.h"
#include "hid_u2f_send.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


bool hid_u2f_command_on_mainsw_event(void)
{
    // NOP
    return true;
}

bool hid_u2f_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

static void u2f_hid_init_do_process(void)
{
    generate_hid_init_response();
    send_hid_input_report(CID_for_session, CMD_for_session, u2f_response_buffer, u2f_response_length);
}

static void u2f_version_do_process(void)
{
    generate_u2f_version_response();
    send_hid_input_report(CID_for_session, CMD_for_session, u2f_response_buffer, u2f_response_length);
}

static void u2f_register_do_process(void)
{
    // これは仮コードです。
    generate_u2f_register_response();
    send_hid_input_report(CID_for_session, CMD_for_session, u2f_response_buffer, u2f_response_length);
}

static void u2f_authenticate_do_process(void)
{
    // これは仮コードです。
    generate_u2f_authenticate_response();
    send_hid_input_report(CID_for_session, CMD_for_session, u2f_response_buffer, u2f_response_length);
}

void hid_u2f_command_on_report_received(void)
{
    uint8_t  ins;

    // データ受信後に実行すべき処理を判定
    switch (CMD_for_session) {
        case U2FHID_INIT:
            u2f_hid_init_do_process();
            break;
            
        case U2FHID_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = u2f_request_buffer[1];
            if (ins == U2F_VERSION) {
                u2f_version_do_process();
                
            } else if (ins == U2F_REGISTER) {
                u2f_register_do_process();
                
            } else if (ins == U2F_AUTHENTICATE) {
                u2f_authenticate_do_process();
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // Flash ROM更新後に行われる後続処理を実行
    UNUSED_PARAMETER(p_evt);
}
