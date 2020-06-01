/* 
 * File:   ccid_apdu.c
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#include "ccid.h"
#include "ccid_apdu.h"

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

//
// 受信したAPDU（Command APDU）を保持
//
static uint8_t  capdu_cla;
static uint8_t  capdu_ins;
static uint8_t  capdu_p1;
static uint8_t  capdu_p2;
static size_t   capdu_lc;
static size_t   capdu_le;
static uint8_t *capdu_data;
static size_t   capdu_data_size;

//
// 送信するAPDU（Response APDU）を保持
//
static uint8_t *rapdu_data;
static uint16_t rapdu_len;
static uint16_t rapdu_sw;

static bool parse_capdu(void) 
{
    // 受信APDUの先頭アドレス、長さを取得
    uint8_t *cmd = ccid_command_apdu_data();
    uint16_t len = ccid_command_apdu_size();
    if (len < 4) {
        // ヘッダー長に満たない場合は終了
        return false;
    }

    // ヘッダー部からコマンドを取得
    capdu_cla = cmd[0];
    capdu_ins = cmd[1];
    capdu_p1 = cmd[2];
    capdu_p2 = cmd[3];
    capdu_lc = 0;
    capdu_le = 0;

    if (len == 4) {
        // APDUデータなし
        // Lc = 未指定
        // Le = 未指定
        return true;
    }

    if (len == 5) {
        // APDUデータなし
        // Lc = 未指定
        // Le = 1 byte encodingで指定あり
        capdu_le = cmd[4];
        capdu_lc = 0;
        if (capdu_le == 0) {
            capdu_le = 0x100;
        }
        return true;
    }

    if (cmd[4] > 0) {
        // Lc が 1 byte encoding指定である場合
        capdu_lc = cmd[4];
        if (len == 5 + capdu_lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 未指定
            //memmove(capdu_data, cmd + 5, capdu_lc);
            capdu_data = cmd + 5;
            capdu_data_size = capdu_lc;
            capdu_le = 0x100;

        } else if (len == 6 + capdu_lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 1 byte encodingで指定あり
            //memmove(capdu_data, cmd + 5, capdu_lc);
            capdu_data = cmd + 5;
            capdu_data_size = capdu_lc;
            capdu_le = cmd[5 + capdu_lc];
            if (capdu_le == 0) {
                capdu_le = 0x100;
            }

        } else {
            return false;
        }

    } else {
        // Lc または Le のいずれかが
        // 3 byte encoding指定である場合
        if (len == 7) {
            // APDUデータなし
            // Lc = 未指定
            // Le = 3 byte encodingで指定あり
            capdu_le = (cmd[5] << 8) | cmd[6];
            if (capdu_le == 0) {
                capdu_le = 0x10000;
            }

        } else {
            capdu_lc = (cmd[5] << 8) | cmd[6];
            if (capdu_lc == 0) {
                return false;
            }
            if (len == 7 + capdu_lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 未指定
                //memmove(capdu_data, cmd + 7, capdu_lc);
                capdu_data = cmd + 7;
                capdu_data_size = capdu_lc;
                capdu_le = 0x10000;

            } else if (len == 9 + capdu_lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 2 byte encodingで指定あり
                //memmove(capdu_data, cmd + 7, capdu_lc);
                capdu_data = cmd + 7;
                capdu_data_size = capdu_lc;
                capdu_le = (cmd[7 + capdu_lc] << 8) | cmd[8 + capdu_lc];
                if (capdu_le == 0) {
                    capdu_le = 0x10000;
                }

            } else {
                return false;
            }
        }
    }
    return true;
}

void ccid_apdu_process(void)
{
    NRF_LOG_DEBUG("APDU received(%d bytes):", ccid_command_apdu_size());
    NRF_LOG_HEXDUMP_DEBUG(ccid_command_apdu_data(), ccid_command_apdu_size());
    
    // 受信したAPDUを解析
    if (parse_capdu() == false) {
        // APDUが不正の場合はエラー扱い
        rapdu_len = 0;
        rapdu_sw = SW_CHECKING_ERROR;

    } else {
        // TODO: 具体的な処理は後日実装
        fido_log_debug("APDU received: CLA(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) Lc(%d bytes) Le(%d bytes)", 
            capdu_cla, capdu_ins, capdu_p1, capdu_p2, capdu_lc, capdu_le);
        fido_log_print_hexdump_debug(capdu_data, capdu_data_size);
    }

    // ステータスワードを設定（APDUの末尾２バイトを使用）
    uint8_t *apdu_data = ccid_response_apdu_data();
    apdu_data[rapdu_len] = HI(rapdu_sw);
    apdu_data[rapdu_len + 1] = LO(rapdu_sw);

    // レスポンスデータに、APDU長を設定
    ccid_response_apdu_size_set(rapdu_len + 2);
}
