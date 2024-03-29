/* 
 * File:   ccid_apdu.c
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#include "ccid.h"
#include "ccid_apdu.h"
#include "ccid_define.h"
#include "ccid_process.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_apdu);
#endif

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
// Flash ROM書込み完了待ち等の事由により、
// レスポンス実行を抑止させるためのフラグ
//
static bool response_is_pending;

bool ccid_apdu_response_is_pending(void)
{
    return response_is_pending;
}

void ccid_apdu_response_set_pending(bool b)
{
    response_is_pending = b;
}

//
// 受信したAPDU（Command APDU）を保持
//
//  全ブロック分を格納
static command_apdu_t command_apdu;
//  １ブロック分を格納
static command_apdu_t capdu_work;
//  全ブロック分のAPDUデータを格納
static uint8_t capdu_data[APDU_BUFFER_SIZE];
// ブロック分割受信中をあらわすフラグ
static bool capdu_merging = false;

//
// 送信するAPDU（Response APDU）を保持
//
static response_apdu_t response_apdu;

static bool parse_command_apdu(command_apdu_t *p_capdu) 
{
    // 受信APDUの先頭アドレス、長さを取得
    uint8_t *cmd = ccid_command_apdu_data();
    uint16_t len = ccid_command_apdu_size();
    if (len < 4) {
        // ヘッダー長に満たない場合は終了
        return false;
    }

    // 受信APDU（Command APDU）
    // １ブロック分の格納領域を初期化
    memset(p_capdu, 0x00, sizeof(command_apdu_t));

    // ヘッダー部からコマンドを取得
    p_capdu->cla = cmd[0];
    p_capdu->ins = cmd[1];
    p_capdu->p1 = cmd[2];
    p_capdu->p2 = cmd[3];
    p_capdu->lc = 0;
    p_capdu->le = 0;

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
        p_capdu->le = cmd[4];
        p_capdu->lc = 0;
        if (p_capdu->le == 0) {
            p_capdu->le = 0x100;
        }
        return true;
    }

    if (cmd[4] > 0) {
        // Lc が 1 byte encoding指定である場合
        p_capdu->lc = cmd[4];
        if (len == 5 + p_capdu->lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 未指定
            //memmove(capdu->data, cmd + 5, capdu->lc);
            p_capdu->data = cmd + 5;
            p_capdu->le = 0x100;

        } else if (len == 6 + p_capdu->lc) {
            // APDUデータあり
            // Lc = 1 byte encodingで指定あり
            // Le = 1 byte encodingで指定あり
            //memmove(capdu->data, cmd + 5, capdu->lc);
            p_capdu->data = cmd + 5;
            p_capdu->le = cmd[5 + p_capdu->lc];
            if (p_capdu->le == 0) {
                p_capdu->le = 0x100;
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
            p_capdu->le = (cmd[5] << 8) | cmd[6];
            if (p_capdu->le == 0) {
                p_capdu->le = 0x10000;
            }

        } else {
            p_capdu->lc = (cmd[5] << 8) | cmd[6];
            if (p_capdu->lc == 0) {
                return false;
            }
            if (len == 7 + p_capdu->lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 未指定
                //memmove(capdu->data, cmd + 7, capdu->lc);
                p_capdu->data = cmd + 7;
                p_capdu->le = 0x10000;

            } else if (len == 9 + p_capdu->lc) {
                // APDUデータあり
                // Lc = 3 bytes encodingで指定あり
                // Le = 2 byte encodingで指定あり
                //memmove(capdu->data, cmd + 7, capdu->lc);
                p_capdu->data = cmd + 7;
                p_capdu->le = (cmd[7 + p_capdu->lc] << 8) | cmd[8 + p_capdu->lc];
                if (p_capdu->le == 0) {
                    p_capdu->le = 0x10000;
                }

            } else {
                return false;
            }
        }
    }
    return true;
}

static void generate_response_status(response_apdu_t *rapdu)
{
    // データバイトの参照
    uint8_t *apdu_data = ccid_response_apdu_data();

    // ステータスワードバイトを格納
    uint16_t sw = rapdu->sw;
    apdu_data[0] = HI(sw);
    apdu_data[1] = LO(sw);

    // APDU長を設定
    // （ステータスワードの２バイト）
    ccid_response_apdu_size_set(2);

    // 送信APDUレスポンスのログ
    fido_log_debug("APDU send: SW(%04x)", sw);
}

static void generate_response_apdu(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    //
    // レスポンスAPDUを生成し、
    // CCID I/F出力(BULK IN)バッファに格納
    //
    // LL=0（ステータス出力）の場合はステータスワードのみレスポンス
    if (rapdu->len == 0) {
        generate_response_status(rapdu);
        return ;
    }

    // Leで指定されたサイズを、最大送信可能バイト数とする
    size_t max_size = (capdu->le < APDU_BUFFER_SIZE) ? capdu->le : APDU_BUFFER_SIZE;

    // 今回送信するフレームサイズを計算
    uint16_t size_to_send = rapdu->len - rapdu->already_sent;
    if (size_to_send > max_size) {
        size_to_send = max_size;
    }

    // データバイトを格納（APDUの先頭からコピー）
    uint8_t *apdu_data = ccid_response_apdu_data();
    memcpy(apdu_data, rapdu->data + rapdu->already_sent, size_to_send);

    // APDU長を設定
    // （ステータスワードの２バイト分を含む）
    ccid_response_apdu_size_set(size_to_send + 2);

    // 送信済みバイト数を更新
    rapdu->already_sent += size_to_send;

    // ステータスワードを生成
    uint16_t sw;
    if (rapdu->already_sent < rapdu->len) {
        if (rapdu->len - rapdu->already_sent > 0xff) {
            sw = 0x61ff;
        } else {
            sw = 0x6100 + (rapdu->len - rapdu->already_sent);
        }
    } else {
        sw = rapdu->sw;
    }

    // ステータスワードバイトを格納（APDUの末尾２バイトを使用）
    apdu_data[size_to_send] = HI(sw);
    apdu_data[size_to_send + 1] = LO(sw);

    // 送信APDUレスポンスのログ
    if (capdu->ins == INS_GET_RESPONSE_APDU) {
        fido_log_debug("APDU send: SW(%04x) data(%d, total %d)", 
            sw, size_to_send, rapdu->len);
    } else {
        fido_log_debug("APDU send: SW(%04x) data(%d)", 
            sw, rapdu->len);
    }
}

static void initialize_all_block_apdu(command_apdu_t *capdu_merged, command_apdu_t *capdu_work)
{
    // 受信APDU（Command APDU）を初期化
    memset(capdu_merged, 0x00, sizeof(command_apdu_t));
    memset(capdu_data, 0x00, sizeof(capdu_data));
    capdu_merged->data = capdu_data;
    capdu_merged->lc = 0;

    // APDUヘッダーを初期化
    capdu_merged->cla = capdu_work->cla & 0xEF;
    capdu_merged->ins = capdu_work->ins;
    capdu_merged->p1 = capdu_work->p1;
    capdu_merged->p2 = capdu_work->p2;
}

static bool is_apdu_block_control_break(command_apdu_t *capdu_merged, command_apdu_t *capdu_work)
{
    uint8_t cla_work = capdu_work->cla & 0xEF;
    if (capdu_merged->cla != cla_work) {
        return true;
    }
    if (capdu_merged->ins != capdu_work->ins) {
        return true;
    }
    if (capdu_merged->p1 != capdu_work->p1) {
        return true;
    }
    if (capdu_merged->p2 != capdu_work->p2) {
        return true;
    }
    return false;
}

static bool merge_command_apdu(command_apdu_t *capdu_merged, command_apdu_t *capdu_work, response_apdu_t *rapdu)
{
    if (capdu_merging == false) {
        // 最初のブロックとして扱う
        initialize_all_block_apdu(capdu_merged, capdu_work);

    } else if (is_apdu_block_control_break(capdu_merged, capdu_work)) {
        // コントロールブレーク判定し、最初のブロックとして扱う
        initialize_all_block_apdu(capdu_merged, capdu_work);
    }

    // ブロック受信中フラグを設定
    capdu_merging = true;
    if (capdu_merged->lc + capdu_work->lc > sizeof(capdu_data)) {
        // オーバーフローしてしまった場合は
        // 処理を行わず、エラーレスポンス
        rapdu->len = 0;
        rapdu->sw = SW_CHECKING_ERROR;
        return false;
    }

    // APDUデータを格納領域にコピー
    memcpy(capdu_data + capdu_merged->lc, capdu_work->data, capdu_work->lc);
    capdu_merged->lc += capdu_work->lc;

    if (capdu_work->cla & 0x10) {
        // 継続ブロック受信時は処理を行わず、正常レスポンス
        rapdu->len = 0;
        rapdu->sw = SW_NO_ERROR;
        return false;
    }

    // 最終ブロック受信時は、業務処理を継続
    capdu_merging = false;
    capdu_merged->le = (capdu_work->le < APDU_BUFFER_SIZE) ? capdu_work->le : APDU_BUFFER_SIZE;

    command_apdu_t *capdu = capdu_merged;
    fido_log_debug("APDU recv: CLA INS P1 P2(%02x %02x %02x %02x) Lc(%d) Le(%d)", 
        capdu->cla, capdu->ins, capdu->p1, capdu->p2, capdu->lc, capdu->le);

    return true;
}

static void get_response_or_process_applet(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // 受信APDUコマンドに対応する処理を実行
#if LOG_DEBUG_APDU_DATA_BUFF
    fido_log_debug("APDU recv: CLA INS P1 P2(%02x %02x %02x %02x) Lc(%d) Le(%d)", 
        capdu->cla, capdu->ins, capdu->p1, capdu->p2, capdu->lc, capdu->le);
    print_hexdump_debug(capdu->data, capdu->data_size);
#endif
    if ((capdu->cla == 0x80 || capdu->cla == 0x00) && capdu->ins == INS_GET_RESPONSE_APDU) {
        // GET RESPONSEの場合、
        // レスポンスAPDUを生成するだけなので、
        // ここでは何も行わない
        return;
    }

    // 受信APDUのブロック分割受信を行う
    command_apdu_t *capdu_merged = &command_apdu;
    if (merge_command_apdu(capdu_merged, capdu, rapdu) == false) {
        // ブロックが継続する場合は
        // レスポンスAPDUを生成するだけなので、
        // ここでは何も行わない
        return;
    }

    // 送信APDU（Response APDU）を初期化
    memset(rapdu, 0x00, sizeof(response_apdu_t));

    // Applet処理を実行
    ccid_process_applet(capdu_merged, rapdu);
}

void ccid_apdu_process(void)
{
    command_apdu_t *capdu = &capdu_work;
    response_apdu_t *rapdu = &response_apdu;
#if LOG_DEBUG_APDU_BUFF
    fido_log_debug("APDU received(%d bytes):", ccid_command_apdu_size());
    print_hexdump_debug(ccid_command_apdu_data(), ccid_command_apdu_size());
#endif

    // レスポンス実行抑止フラグを初期化
    ccid_apdu_response_set_pending(false);

    // 受信したAPDUを解析
    if (parse_command_apdu(capdu) == false) {
        // APDUが不正の場合はエラー扱い
        rapdu->len = 0;
        rapdu->sw = SW_CHECKING_ERROR;

    } else {
        // 受信APDUコマンドに対応する処理を実行
        get_response_or_process_applet(capdu, rapdu);
    }

    // レスポンスAPDUの送信を指示
    ccid_apdu_resume_process(capdu, rapdu);
}

void ccid_apdu_resume_process(void *p_capdu, void *p_rapdu)
{
    if (ccid_apdu_response_is_pending()) {
        // レスポンス実行抑止フラグ設定時は終了
        return;
    }

    command_apdu_t  *capdu = (command_apdu_t *)p_capdu;
    response_apdu_t *rapdu = (response_apdu_t *)p_rapdu;

#if LOG_DEBUG_APDU_DATA_BUFF
    // 送信APDUレスポンスのログ
    fido_log_debug("APDU to send: SW(0x%04x) data(%d bytes)", rapdu->sw, rapdu->len);
    print_hexdump_debug(rapdu->data, rapdu->len);
#endif

    // レスポンスAPDUを生成
    generate_response_apdu(capdu, rapdu);
    
    // レスポンスAPDUの送信を指示
    ccid_resume_reader_to_pc_data_block();
}

void ccid_apdu_assert(void *p_capdu, void *p_rapdu)
{
    if (p_capdu == NULL || p_rapdu == NULL) {
        while(true);
    }
}
