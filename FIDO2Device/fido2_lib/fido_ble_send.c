/* 
 * File:   fido_ble_send.c
 * Author: makmorit
 *
 * Created on 2019/06/27, 9:49
 */
//
// プラットフォーム非依存コード
//
#include "u2f.h"
#include "fido_ble_command.h"
#include "fido_ble_receive.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug hex dump data
#define NRF_LOG_HEXDUMP_DEBUG_PACKET false

// u2f_status（レスポンスバッファ）には、
// 64バイトまで書込み可能とします
#define BLE_U2F_MAX_SEND_CHAR_LEN 64
static uint8_t  u2f_status_buffer[BLE_U2F_MAX_SEND_CHAR_LEN];
static uint16_t u2f_status_buffer_length;

static struct {
    // 送信済みバイト数、シーケンスを保持
    uint32_t     sent_length;
    uint8_t      sequence;

    // 送信用BLEヘッダーに格納するコマンド、データ長、
    // 送信データを保持
    uint8_t      command_for_response;
    uint8_t     *data;
    uint32_t     data_length;

    // BLE送信がビジー状態かどうかを保持するフラグ
    bool         busy;
} send_info_t;


static uint8_t edit_u2f_staus_header(uint8_t command, uint32_t length, uint8_t sequence)
{
    // u2f_staus_headerにおける
    // データの開始位置
    uint8_t offset;

    // 領域をクリア
    memset(u2f_status_buffer, 0, sizeof(u2f_status_buffer));

    if (sequence == 0) {
        // 先頭パケットの場合はBLEヘッダー項目を設定
        //   コマンド
        //   データ（APDUまたはPINGパケット）長
        u2f_status_buffer[0] = command;
        u2f_status_buffer[1] = (uint8_t)(length >> 8 & 0x000000FF);
        u2f_status_buffer[2] = (uint8_t)(length >> 0 & 0x000000FF);
        offset = 3;

    } else {
        // 後続パケットの場合はシーケンス番号を設定
        u2f_status_buffer[0] = sequence - 1;
        offset = 1;
    }

    return offset;
}

static uint8_t edit_u2f_staus_data(uint8_t offset)
{
    // 送信データ（先頭アドレス・長さ）と
    // 送信済みバイト数を取得
    uint8_t  *data_buffer       = send_info_t.data;
    uint32_t data_buffer_length = send_info_t.data_length;
    uint32_t sent_length        = send_info_t.sent_length;

    // 今回送信するデータ部のバイト数
    uint32_t data_length;

    // データの長さを計算
    // (総バイト数 - 送信ずみバイト数)
    uint32_t remaining = data_buffer_length - sent_length;

    // 今回送信するデータ部のバイト数を計算
    u2f_status_buffer_length = remaining + offset;
    if (u2f_status_buffer_length > BLE_U2F_MAX_SEND_CHAR_LEN) {
        u2f_status_buffer_length = BLE_U2F_MAX_SEND_CHAR_LEN;
    }
    data_length = u2f_status_buffer_length - offset;

    // データ部をセット
    memcpy(u2f_status_buffer + offset, data_buffer + sent_length, data_length);

    return data_length;
}


static void ble_u2f_status_setup(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length)
{
    // 送信のために必要な情報を保持
    send_info_t.command_for_response = command_for_response;
    send_info_t.data = data_buffer;
    send_info_t.data_length = data_buffer_length;

    // 送信済みバイト数、シーケンスをゼロクリア
    send_info_t.sent_length = 0;
    send_info_t.sequence = 0;

    // フラグをクリア
    send_info_t.busy = false;
}

static uint32_t ble_u2f_status_response_send(void)
{
    uint32_t data_length;
    uint32_t err_code;

    // フラグがビジーの場合は異常終了
    if (send_info_t.busy == true) {
        fido_log_error("ble_u2f_status_response_send: function is busy ");
        return NRF_ERROR_INVALID_STATE;
    }

    // 保持中の情報をチェックし、
    // 完備していない場合は異常終了
    if (send_info_t.data == NULL) {
        fido_log_error("ble_u2f_status_response_send: ble_u2f_status_setup incomplete ");
        return NRF_ERROR_INVALID_DATA;
    }

    while (send_info_t.sent_length < send_info_t.data_length) {

        // ヘッダー項目、データ部を編集
        uint8_t offset = edit_u2f_staus_header(send_info_t.command_for_response, send_info_t.data_length, send_info_t.sequence);
        data_length = edit_u2f_staus_data(offset);

        // u2f_status_bufferに格納されたパケットを送信
        err_code = fido_ble_response_send(u2f_status_buffer, u2f_status_buffer_length);
        if (err_code != NRF_SUCCESS) {

            if (err_code == NRF_ERROR_RESOURCES) {
                // 未送信データが存在する状態の場合は
                // ビジーと判断して送信中断。
                // イベントBLE_GATTS_EVT_HVN_TX_COMPLETEが
                // 通知されたら、本関数を再度呼び出して再送させる。
                send_info_t.busy = true;
            } else if (err_code == NRF_ERROR_INVALID_STATE) {
                // "ATT_MTU exchange is ongoing"状態と判断して送信中断。
                // リトライタイマーをスタートし、３秒後、本関数を再度呼び出して再送させる。
                fido_ble_send_retry_timer_start();
            }

            break;
        }

        // 送信済みバイト数、シーケンスを更新
        send_info_t.sent_length += data_length;
        send_info_t.sequence++;

        // 最終レコードの場合は、次回リクエストまでの経過秒数監視をスタート
        if (send_info_t.sent_length == send_info_t.data_length) {
            // FIDOレスポンス送信完了時の処理を実行
            fido_ble_command_on_response_send_completed();
        }
    }

    return err_code;
}


void fido_ble_send_on_tx_complete(void)
{
    if (send_info_t.busy == true) {
        // フラグがbusyの場合、再送のため１回だけ
        // ble_u2f_status_response_send関数を呼び出す
        send_info_t.busy = false;
        ble_u2f_status_response_send();
    }
}


void fido_ble_send_response_retry(void)
{
    // U2Fクライアントとの接続が切り離された時は終了
    if (fido_ble_service_disconnected()) {
        return;
    }
    
    // レスポンスを送信
    uint32_t err_code = ble_u2f_status_response_send();
    fido_log_debug("ble_u2f_status_response_send retry: err_code=0x%02x ", err_code);
}

//
// メッセージ送信関連処理
//
// レスポンス編集エリア
static uint8_t  data_buffer[2];
static uint32_t data_buffer_length;

void fido_ble_send_response_data(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length)
{
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send();
}

void fido_ble_send_success_response(uint8_t command_for_response)
{
    // レスポンスを生成
    fido_set_status_word(data_buffer, U2F_SW_NO_ERROR);
    data_buffer_length = 2;

    // 生成したレスポンスを戻す
    fido_ble_send_response_data(command_for_response, data_buffer, data_buffer_length);
}

void fido_ble_send_error_response(uint8_t command_for_response, uint16_t err_status_word)
{    
    // ステータスワードを格納
    fido_set_status_word(data_buffer, err_status_word);
    data_buffer_length = 2;
    
    // レスポンスを送信
    fido_ble_send_response_data(command_for_response, data_buffer, data_buffer_length);
}

void fido_ble_send_command_error_response(uint8_t err_code)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_ERROR;

    // エラーコードを格納
    data_buffer[0]     = err_code;
    data_buffer_length = 1;

    // レスポンスを送信
    fido_ble_send_response_data(command_for_response, data_buffer, data_buffer_length);
}

void fido_ble_send_keepalive_response(uint8_t keepalive_status_byte)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_KEEPALIVE;

    // エラーコードを格納
    data_buffer[0]     = keepalive_status_byte;
    data_buffer_length = 1;

    // レスポンスを送信
    fido_ble_send_response_data(command_for_response, data_buffer, data_buffer_length);
}

void fido_ble_send_ping_response(void)
{
    // BLE接続情報、BLEヘッダー、APDUの参照を取得
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T *p_apdu = fido_ble_receive_apdu();

    // PINGの場合は
    // リクエストのBLEヘッダーとデータを編集せず
    // レスポンスとして戻す（エコーバック）
    fido_ble_send_response_data(p_ble_header->CMD, p_apdu->data, p_apdu->data_length);
}
