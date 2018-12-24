#include "sdk_common.h"

#include "ble_u2f_util.h"
#include "ble_u2f_control_point_apdu.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_control_point
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// u2f control point（コマンドバッファ）には、
// 64バイトまで書込み可能とします
static uint8_t  control_point_buffer[BLE_U2F_MAX_RECV_CHAR_LEN];
static uint16_t control_point_buffer_length;

// リクエストデータに含まれる
// BLEヘッダー、APDU項目は
// このモジュール内で保持
static BLE_HEADER_T ble_header_t;
static FIDO_APDU_T   apdu_t;


void ble_u2f_control_point_initialize(void)
{
    // コマンド／リクエストデータ格納領域を初期化
    memset(control_point_buffer, 0x00, BLE_U2F_MAX_RECV_CHAR_LEN);
    control_point_buffer_length = 0;
    memset(&ble_header_t, 0x00, sizeof(BLE_HEADER_T));
    memset(&apdu_t, 0x00, sizeof(FIDO_APDU_T));
}

static bool is_valid_command(uint8_t command)
{
    if (command == U2F_COMMAND_PING) {
        return true;
    } else if (command == U2F_COMMAND_MSG) {
        return true;
    } else {
        return false;
    }
}

static bool u2f_request_receive_leading_packet(ble_u2f_context_t *p_u2f_context, BLE_HEADER_T *p_ble_header, FIDO_APDU_T *p_apdu)
{
    if (control_point_buffer_length < 3) {
        // 受取ったバイト数が３バイトに満たない場合は、
        // リクエストとして成立しないので終了
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        NRF_LOG_ERROR("u2f_request_receive: invalid request ");
        return false;
    }

    // BLEヘッダー項目を保持
    p_ble_header->CMD = control_point_buffer[0];
    // データ（APDUまたはPINGパケット）の長さを取得
    p_ble_header->LEN = (uint32_t)(
                         (control_point_buffer[1] << 8 & 0xFF00) 
                        + control_point_buffer[2]);
    p_ble_header->SEQ = 0xff;

    NRF_LOG_DEBUG("INIT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    if (is_valid_command(p_ble_header->CMD) == false) {
        // BLEヘッダーに設定されたコマンドが不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("u2f_request_receive: invalid command (0x%02x) ", p_ble_header->CMD);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_COMMAND;
        return false;
    }

    if (p_ble_header->LEN > BLE_U2F_MAX_RECV_CHAR_LEN - 3) {
        // BLEヘッダーに設定されたデータ長が
        // 61文字を超える場合、後続データがあると判断
        NRF_LOG_DEBUG("u2f_request_receive: CONT frame will receive ");
        p_ble_header->CONT = true;
    } else {
        p_ble_header->CONT = false;
    }
    
    // BLEヘッダーだけの場合は、ここで処理を終了
    if (control_point_buffer_length == 3) {
        return true;
    }

    // Control Point参照用の先頭インデックス
    // （＝処理済みバイト数）を保持
    int offset = 3;

    if (p_ble_header->CMD == U2F_COMMAND_PING) {
        // コマンドがPINGの場合
        // データ長だけセットしておく
        p_apdu->Lc = p_ble_header->LEN;
    } else {
        // コマンドがPING以外の場合
        // APDUヘッダー項目を編集して保持
        offset += ble_u2f_control_point_apdu_header(p_apdu, control_point_buffer, control_point_buffer_length, offset);
    }

    if (p_apdu->Lc > APDU_DATA_MAX_LENGTH) {
        // ヘッダーに設定されたデータ長が不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("u2f_request_receive: too long length (%d) ", p_apdu->Lc);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        return false;
    }

    if (p_apdu->Lc == 0) {
        if (offset < control_point_buffer_length) {
            // データ長が0にもかかわらず、
            // APDUヘッダーの後ろにデータが存在している場合、
            // リクエストとしては不正ではないが、
            // ステータスワード(SW_WRONG_LENGTH)を設定
            NRF_LOG_ERROR("INIT frame has data (%d bytes) while Lc=0 ", control_point_buffer_length - offset);
            p_ble_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        }
        // データ長が0の場合は以降の処理を行わない
        return true;
    }

    if (ble_u2f_control_point_apdu_allocate(p_u2f_context, p_apdu) == false) {
        // データ格納領域を初期化し、アドレスを保持
        return false;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    ble_u2f_control_point_apdu_from_leading(p_apdu, control_point_buffer, control_point_buffer_length, offset);

    return true;
}


static void u2f_request_receive_following_packet(BLE_HEADER_T *p_ble_header, FIDO_APDU_T *p_apdu)
{
    // CMDが空の場合は先頭レコード未送信とみなし
    // エラーと扱う
    if (p_ble_header->CMD == 0x00) {
        NRF_LOG_ERROR("INIT frame not received ");

        ble_header_t.CMD = U2F_COMMAND_ERROR;
        ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
        return;
    }

    // SEQには、分割受信時の２番目以降の
    // レコード連番が入ります
    uint8_t sequence = control_point_buffer[0];

    // シーケンスチェック
    if (sequence == 0) {
        if (p_ble_header->SEQ != 0xff) {
            NRF_LOG_ERROR("Irregular 1st sequence %d ", sequence);

            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    } else {
        if (sequence != p_ble_header->SEQ+1) {
            NRF_LOG_ERROR("Bad sequence %d-->%d ", 
                p_ble_header->SEQ, sequence);

            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    }

    // シーケンスを更新
    p_ble_header->SEQ = sequence;

    NRF_LOG_DEBUG("CONT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    // コピー先となる領域が初期化されていない場合は終了
    if (p_apdu->data == NULL) {
        NRF_LOG_ERROR("p_apdu->data is not initialized ");
        return;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    ble_u2f_control_point_apdu_from_following(p_ble_header, p_apdu, control_point_buffer, control_point_buffer_length);
}

void ble_u2f_control_point_receive(ble_gatts_evt_write_t *p_evt_write, ble_u2f_context_t *p_u2f_context)
{
    // U2Fクライアントから受信したリクエストデータを、
    // 内部バッファに保持
    memset(control_point_buffer, 0, sizeof(control_point_buffer));
    memcpy(control_point_buffer, p_evt_write->data, p_evt_write->len);
    control_point_buffer_length = p_evt_write->len;

    NRF_LOG_DEBUG("ble_u2f_control_point_receive length=%u ", control_point_buffer_length);
    NRF_LOG_HEXDUMP_DEBUG(control_point_buffer, control_point_buffer_length);

    if (control_point_buffer[0] & 0x80) {
        // 先頭データが２回連続で送信された場合はエラー
        if ((ble_header_t.CMD & 0x80) && ble_header_t.CONT == true) {
            NRF_LOG_ERROR("INIT frame received again while CONT is expected ");
            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;

        } else {
            // BLEヘッダーとAPDUを初期化
            memset(&ble_header_t, 0, sizeof(BLE_HEADER_T));
            memset(&apdu_t, 0, sizeof(FIDO_APDU_T));

            // 先頭パケットに対する処理を行う
            u2f_request_receive_leading_packet(
                p_u2f_context, &ble_header_t, &apdu_t);
        }

    } else {
        // 後続パケットに対する処理を行う
        u2f_request_receive_following_packet(&ble_header_t, &apdu_t);
        ble_header_t.CONT = false;
    }

    if (apdu_t.data_length > apdu_t.Lc) {
        // データヘッダー設定されたデータ長が不正の場合
        // エラーレスポンスメッセージを作成
        NRF_LOG_ERROR("apdu data length(%d) exceeds Lc(%d) ", apdu_t.data_length, apdu_t.Lc);
        ble_header_t.CMD = U2F_COMMAND_ERROR;
        ble_header_t.ERROR = CTAP1_ERR_INVALID_LENGTH;
    }

    // 共有情報にBLEヘッダーとAPDUの参照を引き渡す
    p_u2f_context->p_ble_header = &ble_header_t;
    p_u2f_context->p_apdu = &apdu_t;
}
