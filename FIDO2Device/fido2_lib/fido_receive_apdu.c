/* 
 * File:   fido_receive_apdu.c
 * Author: makmorit
 *
 * Created on 2018/11/29, 9:57
 */
#include <string.h>
//
// プラットフォーム非依存コード
//
#include "fido_common.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_receive_apdu);
#endif

// for debug receiving data
#define LOG_DEBUG_RECEIVING false

// FIDO機能リクエストAPDU編集用の作業領域（固定長）
static uint8_t apdu_data_buffer[APDU_DATA_MAX_LENGTH];

static uint16_t get_apdu_lc_value(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
{
    // Leの先頭バイトの値を参照し
    // APDUのエンコード種類を判定
    uint16_t lc_length;
    if (control_point_buffer[offset] == 0) {
        // Lcバイト数は3バイト
        lc_length = 3;

        // Extended Length Encoding と扱い、
        // データの長さを取得
        uint32_t length = (uint32_t)(
            (control_point_buffer[offset+1] << 8 & 0xFF00) +
             control_point_buffer[offset+2]);

        if (control_point_buffer_length == (offset + lc_length)) {
            // Lcバイトが存在しない場合は
            // Leバイトと扱う
            p_apdu->Lc = 0;
            if (length == 0) {
                // Leが0の場合は65536と扱う
                p_apdu->Le = 65536;
            } else {
                p_apdu->Le = length;
            }
            fido_log_debug("Lc(%d bytes) Le(%d bytes) in Extended Length Encoding", p_apdu->Lc, p_apdu->Le);
        } else {
            // 先頭パケットからはLcの値だけしか取得できない
            // Leの値は最終パケットから取得する
            p_apdu->Lc = length;
            fido_log_debug("Lc(%d bytes) in Extended Length Encoding", p_apdu->Lc);
        }

    } else {
        // Lcバイト数は1バイト
        lc_length = 1;

        // Short Encoding と扱い、
        // データの長さを取得
        uint32_t length = (uint32_t)control_point_buffer[offset];

        if (control_point_buffer_length == (offset + lc_length)) {
            // Lcバイトが存在しない場合は
            // Leバイトと扱う
            p_apdu->Lc = 0;
            if (length == 0) {
                // Leが0の場合は256と扱う
                p_apdu->Le = 256;
            } else {
                p_apdu->Le = length;
            }
            fido_log_debug("Lc(%d bytes) Le(%d bytes) in Short Encoding", p_apdu->Lc, p_apdu->Le);
        } else {
            // 先頭パケットからはLcの値だけしか取得できない
            // Leの値は最終パケットから取得する
            p_apdu->Lc = length;
            fido_log_debug("Lc(%d bytes) in Short Encoding", p_apdu->Lc);
        }
    }

    return lc_length;
}

uint8_t fido_receive_apdu_header(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
{
    uint8_t apdu_header_length = 4;
    
    // APDUヘッダー項目を保持
    p_apdu->CLA = control_point_buffer[offset];
    p_apdu->INS = control_point_buffer[offset + 1];
    p_apdu->P1  = control_point_buffer[offset + 2];
    p_apdu->P2  = control_point_buffer[offset + 3];

    fido_log_debug("CLA(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) ", 
        p_apdu->CLA, p_apdu->INS, p_apdu->P1, p_apdu->P2);

    // APDUヘッダーだけの場合はここで終了
    offset += apdu_header_length;
    if (control_point_buffer_length == offset) {
        return apdu_header_length;
    }

    // Lcの値をAPDUから取得する
    uint16_t lc_length = get_apdu_lc_value(p_apdu, control_point_buffer, control_point_buffer_length, offset);
    
    // (APDUヘッダー長+LCバイト長)を戻す
    return apdu_header_length + lc_length;
}

void fido_receive_apdu_initialize(FIDO_APDU_T *p_apdu)
{
    // 確保領域は0で初期化
    memset(apdu_data_buffer, 0, APDU_DATA_MAX_LENGTH);

    // 確保領域のアドレスをAPDU情報にも保持
    p_apdu->data = apdu_data_buffer;
}

static uint16_t get_apdu_le_value(FIDO_APDU_T *p_apdu, uint8_t *received_data, uint16_t received_data_length)
{
    // Leのバイト数を求める
    uint16_t le_length = (p_apdu->data_length + received_data_length) - p_apdu->Lc;

    if (le_length == 2) {
        // Leバイトが2バイトの場合
        // Extended Length Encoding と扱い、データの長さを
        // control_point_bufferの末尾２バイトから取得
        p_apdu->Le = (uint32_t)(
            (received_data[received_data_length-2] << 8 & 0xFF00) +
             received_data[received_data_length-1]);
        if (p_apdu->Le == 0) {
            p_apdu->Le = 65536;
        }
        fido_log_debug("Le(%d bytes) in Extended Length Encoding ", p_apdu->Le);

    } else if (le_length == 1) {
        // Leバイトが1バイトの場合
        // Short Encoding と扱い、データの長さを
        // control_point_bufferの最終バイトから取得
        p_apdu->Le = (uint32_t)received_data[received_data_length-1];
        if (p_apdu->Le == 0) {
            p_apdu->Le = 256;
        }
        fido_log_debug("Le(%d bytes) in Short Encoding ", p_apdu->Le);

    } else {
        // エンコーディングルールに反している場合はエラーとして長さ0を戻す
        fido_log_debug("Le(%d bytes) in Unknown Encoding ", le_length);
        le_length = 0;
    }
    
    return le_length;
}

void fido_receive_apdu_from_init_frame(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
{
    // Control Pointに格納されている
    // 受信データの先頭アドレスとデータ長を取得
    uint8_t *received_data        = control_point_buffer + offset;
    int      received_data_length = control_point_buffer_length - offset;

    if (received_data_length > p_apdu->Lc) {
        // データの先頭パケットだが、データ長をオーバーしている場合、
        // オーバーした部分はLeバイトとして扱い、
        // Leバイトを除いた部分を、データ部として扱う
        uint16_t le_length = get_apdu_le_value(p_apdu, received_data, received_data_length);
        received_data_length = received_data_length - le_length;
    }

    // データを格納し、格納データのバイト数を保持
    memcpy(p_apdu->data, received_data, received_data_length);
    p_apdu->data_length = received_data_length;

#if LOG_DEBUG_RECEIVING
    if (p_apdu->data_length < p_apdu->Lc) {
        fido_log_debug("recv INIT frame: received data (%d of %d) ", p_apdu->data_length, p_apdu->Lc);
    } else {
        fido_log_debug("recv INIT frame: received data (%d bytes) ", p_apdu->data_length);
    }
#endif
}

void fido_receive_apdu_from_cont_frame(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length)
{
    // 受信データの先頭アドレスとデータ長を取得
    uint8_t *received_data        = control_point_buffer + 1;
    uint16_t received_data_length = control_point_buffer_length - 1;

    if (p_apdu->data_length + received_data_length > p_apdu->Lc) {
        // データの最終パケットだが、データ長をオーバーしている場合、
        // オーバーした部分はLeバイトとして扱い、
        // Leバイトを除いた部分を、データ部として扱う
        uint16_t le_length = get_apdu_le_value(p_apdu, received_data, received_data_length);
        received_data_length = received_data_length - le_length;
    }

    // コピー済みのデータの直後に取得したデータを連結
    memcpy(p_apdu->data + p_apdu->data_length, received_data, received_data_length);
    p_apdu->data_length += received_data_length;

#if LOG_DEBUG_RECEIVING
    if (p_apdu->data_length < p_apdu->Lc) {
        fido_log_debug("recv CONT frame: received data (%d of %d) ", p_apdu->data_length, p_apdu->Lc);
    } else {
        fido_log_debug("recv CONT frame: received data (%d bytes) ", p_apdu->data_length);
    }
#endif
}
