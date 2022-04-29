/* 
 * File:   ccid_process.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 9:07
 */
#include "ccid_apdu.h"
#include "ccid_piv.h"
#include "ccid_openpgp.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_process);
#endif

//
// Applet
//
static CCID_APPLET current_applet;

void ccid_process_stop_applet(void) 
{
    switch (current_applet) {
        case APPLET_PIV:
            ccid_piv_stop_applet();
            break;
        case APPLET_OPENPGP:
            ccid_openpgp_stop_applet();
            break;
        case APPLET_OATH:
            // TODO: 後日実装
            break;
        default:
            break;
    }
}

static bool command_is_applet_selection(command_apdu_t *capdu)
{
    return (capdu->cla == 0x00 && capdu->ins == 0xA4 && capdu->p1 == 0x04 && capdu->p2 == 0x00);
}

static bool select_applet(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    if (ccid_piv_rid_is_piv_applet(capdu)) {
        // PIV
        if (current_applet != APPLET_PIV) {
            ccid_process_stop_applet();
        }
        current_applet = APPLET_PIV;
        fido_log_debug("select_applet: applet switched to PIV");
        return true;

    } else if (ccid_openpgp_aid_is_applet(capdu)) {
        // OpenPGP
        if (current_applet != APPLET_OPENPGP) {
            ccid_process_stop_applet();
        }
        current_applet = APPLET_OPENPGP;
        fido_log_debug("select_applet: applet switched to OpenPGP");
        return true;
    }

    // appletを選択できなかった場合
    rapdu->len = 0;
    rapdu->sw = SW_FILE_NOT_FOUND;
    fido_log_debug("select_applet: applet not found");
    return false;
}

void ccid_process_applet(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    if (command_is_applet_selection(capdu)) {
        if (select_applet(capdu, rapdu) == false) {
            return;
        }
    }
    switch (current_applet) {
        case APPLET_PIV:
            ccid_piv_apdu_process(capdu, rapdu);
            break;
        case APPLET_OPENPGP:
            ccid_openpgp_apdu_process(capdu, rapdu);
            break;
        default:
            rapdu->len = 0;
            rapdu->sw = SW_FILE_NOT_FOUND;
            break;
    }
}
