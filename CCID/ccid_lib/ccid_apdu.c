/* 
 * File:   ccid_apdu.c
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#include "ccid.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// Applet
//
enum APPLET {
  APPLET_NONE,
  APPLET_PIV,
  APPLET_OATH,
  APPLET_OPENPGP
};
static enum APPLET current_applet;

void ccid_apdu_stop_applet(void) 
{
    switch (current_applet) {
        case APPLET_PIV:
            // TODO: 後日実装
            break;
        case APPLET_OATH:
            // TODO: 後日実装
            break;
        case APPLET_OPENPGP:
            // TODO: 後日実装
            break;
        default:
            break;
    }
}

void ccid_apdu_process(void)
{
    NRF_LOG_DEBUG("APDU received(%d bytes):", ccid_command_apdu_size());
    NRF_LOG_HEXDUMP_DEBUG(ccid_command_apdu_data(), ccid_command_apdu_size());
}
