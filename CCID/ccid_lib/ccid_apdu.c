/* 
 * File:   ccid_apdu.c
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#include "ccid.h"
#include "ccid_piv.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug apdu data
#define LOG_DEBUG_APDU_BUFF       false
#define LOG_DEBUG_APDU_DATA_BUFF  false
#define LOG_DEBUG_BUFF (LOG_DEBUG_APDU_BUFF || LOG_DEBUG_APDU_DATA_BUFF)

#if LOG_DEBUG_BUFF
static void print_hexdump_debug(uint8_t *buff, size_t size)
{
    int j, k;
    for (j = 0; j < size; j += 64) {
        k = size - j;
        fido_log_print_hexdump_debug(buff + j, (k < 64) ? k : 64);
    }
}
#endif

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

// Appletをあらわすバイト配列
static uint8_t applet_id_piv[] = {0xa0, 0x00, 0x00, 0x03, 0x08};

void ccid_apdu_stop_applet(void) 
{
    switch (current_applet) {
        case APPLET_PIV:
            ccid_piv_stop_applet();
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
static command_apdu_t capdu;

//
// 送信するAPDU（Response APDU）を保持
//
static response_apdu_t rapdu;

static void initialize_apdu_values(void)
{
    // 受信APDU（Command APDU）を初期化
    memset(&capdu, 0x00, sizeof(command_apdu_t));

    // 送信APDU（Response APDU）を初期化
    memset(&rapdu, 0x00, sizeof(response_apdu_t));
    rapdu.data = ccid_response_apdu_data();
}

static bool parse_command_apdu(void) 
{
    // 受信APDUの先頭アドレス、長さを取得
    uint8_t *cmd = ccid_command_apdu_data();
    uint16_t len = ccid_command_apdu_size();
    if (len < 4) {
        // ヘッダー長に満たない場合は終了
        return false;
    }

    // ヘッダー部からコマンドを取得
    capdu.cla = cmd[0];
    capdu.ins = cmd[1];
    capdu.p1 = cmd[2];
    capdu.p2 = cmd[3];
    capdu.lc = 0;
    capdu.le = 0;

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
        capdu.le = cmd[4];
        capdu.lc = 0;
        if (capdu.le == 0) {
            capdu.le = 0x100;
        }
        return true;
    }

    if (cmd[4] > 0) {
        // Lc が 1 byte encoding指定である場合
        capdu.lc = cmd[4];
        if (len == 5 + capdu.lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 未指定
            //memmove(capdu.data, cmd + 5, capdu.lc);
            capdu.data = cmd + 5;
            capdu.data_size = capdu.lc;
            capdu.le = 0x100;

        } else if (len == 6 + capdu.lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 1 byte encodingで指定あり
            //memmove(capdu.data, cmd + 5, capdu.lc);
            capdu.data = cmd + 5;
            capdu.data_size = capdu.lc;
            capdu.le = cmd[5 + capdu.lc];
            if (capdu.le == 0) {
                capdu.le = 0x100;
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
            capdu.le = (cmd[5] << 8) | cmd[6];
            if (capdu.le == 0) {
                capdu.le = 0x10000;
            }

        } else {
            capdu.lc = (cmd[5] << 8) | cmd[6];
            if (capdu.lc == 0) {
                return false;
            }
            if (len == 7 + capdu.lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 未指定
                //memmove(capdu.data, cmd + 7, capdu.lc);
                capdu.data = cmd + 7;
                capdu.data_size = capdu.lc;
                capdu.le = 0x10000;

            } else if (len == 9 + capdu.lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 2 byte encodingで指定あり
                //memmove(capdu.data, cmd + 7, capdu.lc);
                capdu.data = cmd + 7;
                capdu.data_size = capdu.lc;
                capdu.le = (cmd[7 + capdu.lc] << 8) | cmd[8 + capdu.lc];
                if (capdu.le == 0) {
                    capdu.le = 0x10000;
                }

            } else {
                return false;
            }
        }
    }
    return true;
}

static bool command_is_applet_selection(void)
{
    return (capdu.cla == 0x00 && capdu.ins == 0xA4 && capdu.p1 == 0x04 && capdu.p2 == 0x00);
}

static bool select_applet(void)
{
    if (capdu.lc >= sizeof(applet_id_piv) 
        && memcmp(capdu.data, applet_id_piv, sizeof(applet_id_piv)) == 0) {
        // PIV
        if (current_applet != APPLET_PIV) {
            ccid_apdu_stop_applet();
        }
        current_applet = APPLET_PIV;
        fido_log_debug("select_applet: applet switched to PIV");
        return true;
    }

    // appletを選択できなかった場合
    rapdu.len = 0;
    rapdu.sw = SW_FILE_NOT_FOUND;
    fido_log_error("select_applet: applet not found");
    return false;
}

static void process_applet(void)
{
    if (command_is_applet_selection()) {
        if (select_applet() == false) {
            return;
        }
    }
    switch (current_applet) {
        case APPLET_PIV:
            ccid_piv_apdu_process(&capdu, &rapdu);
            break;
        default:
            rapdu.len = 0;
            rapdu.sw = SW_FILE_NOT_FOUND;
            break;
    }
}

void ccid_apdu_process(void)
{
#if LOG_DEBUG_APDU_BUFF
    fido_log_debug("APDU received(%d bytes):", ccid_command_apdu_size());
    print_hexdump_debug(ccid_command_apdu_data(), ccid_command_apdu_size());
#endif

    // APDUデータ保持領域を初期化
    initialize_apdu_values();

    // 受信したAPDUを解析
    if (parse_command_apdu() == false) {
        // APDUが不正の場合はエラー扱い
        rapdu.len = 0;
        rapdu.sw = SW_CHECKING_ERROR;

    } else {
        // 受信APDUコマンドに対応する処理を実行
        fido_log_debug("APDU received: CLA(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) Lc(%d bytes) Le(%d bytes)", 
            capdu.cla, capdu.ins, capdu.p1, capdu.p2, capdu.lc, capdu.le);
#if LOG_DEBUG_APDU_DATA_BUFF
        print_hexdump_debug(capdu.data, capdu.data_size);
#endif
        process_applet();
    }

    // ステータスワードを設定（APDUの末尾２バイトを使用）
    uint8_t *apdu_data = ccid_response_apdu_data();
    apdu_data[rapdu.len] = HI(rapdu.sw);
    apdu_data[rapdu.len + 1] = LO(rapdu.sw);

    // レスポンスデータに、APDU長を設定
    ccid_response_apdu_size_set(rapdu.len + 2);
}
