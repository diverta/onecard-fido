#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include "ble_u2f.h"
#include "ble_u2f_util.h"
#include "ble_u2f_control_point_apdu.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_control_point_apdu"
#include "nrf_log.h"


static uint16_t get_apdu_lc_value(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
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
            NRF_LOG_DEBUG("INIT frame: Extended Length Encoding: Lc(%d) Le(%d) \r\n", p_apdu->Lc, p_apdu->Le);
        } else {
            // 先頭パケットからはLcの値だけしか取得できない
            // Leの値は最終パケットから取得する
            NRF_LOG_DEBUG("INIT frame: Extended Length Encoding: Lc(%d) \r\n", p_apdu->Lc);
            p_apdu->Lc = length;
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
            NRF_LOG_DEBUG("INIT frame: Short Encoding: Lc(%d) Le(%d) \r\n", p_apdu->Lc, p_apdu->Le);
        } else {
            // 先頭パケットからはLcの値だけしか取得できない
            // Leの値は最終パケットから取得する
            p_apdu->Lc = length;
            NRF_LOG_DEBUG("INIT frame: Short Encoding: Lc(%d) \r\n", p_apdu->Lc);
        }
    }

    return lc_length;
}

uint8_t ble_u2f_control_point_apdu_header(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
{
    uint8_t apdu_header_length = 4;
    
    // APDUヘッダー項目を保持
    p_apdu->CLA = control_point_buffer[offset];
    p_apdu->INS = control_point_buffer[offset + 1];
    p_apdu->P1  = control_point_buffer[offset + 2];
    p_apdu->P2  = control_point_buffer[offset + 3];

    NRF_LOG_DEBUG("INIT frame: CLA(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) \r\n", 
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

bool ble_u2f_control_point_apdu_allocate(ble_u2f_context_t *p_u2f_context, U2F_APDU_T *p_apdu)
{
    // APDUのリクエストデータ部を
    // 格納するエリアをヒープに確保する
    uint8_t *apdu_data_buffer = p_u2f_context->apdu_data_buffer;
    if (apdu_data_buffer != NULL) {
        // 既に確保済みの場合
        NRF_LOG_DEBUG("apdu_data_buffer already allocated (%d bytes) \r\n", 
            p_u2f_context->apdu_data_buffer_length);
        memset(apdu_data_buffer, 0, APDU_DATA_MAX_LENGTH);

        // 確保領域のアドレスをAPDU情報にも保持
        p_apdu->data = apdu_data_buffer;
        return true;
    }

    // データ格納領域を、データ全体の長さ分確保
    apdu_data_buffer = (uint8_t *)malloc(APDU_DATA_MAX_LENGTH);
    if (apdu_data_buffer == NULL) {
        NRF_LOG_ERROR("apdu_data_buffer allocation failed \r\n");
        return false;
    }

    // 処理内部で確保したヒープの参照先と確保バイト数を保持
    // (Disconnect時に解放されます)
    p_u2f_context->apdu_data_buffer        = apdu_data_buffer;
    p_u2f_context->apdu_data_buffer_length = APDU_DATA_MAX_LENGTH;
    NRF_LOG_DEBUG("response_message_buffer allocated (%d bytes) \r\n", APDU_DATA_MAX_LENGTH);

    // 確保領域は0で初期化
    memset(apdu_data_buffer, 0, APDU_DATA_MAX_LENGTH);

    // 確保領域のアドレスをAPDU情報にも保持
    p_apdu->data = apdu_data_buffer;

    return true;
}


void ble_u2f_control_point_apdu_from_leading(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset)
{
    // Control Pointに格納されている
    // 受信データの先頭アドレスとデータ長を取得
    uint8_t *data         = control_point_buffer + offset;
    int      data_length  = control_point_buffer_length - offset;

    // データを格納し、格納データのバイト数を保持
    memcpy(p_apdu->data, data, data_length);
    p_apdu->data_length = data_length;

    if (p_apdu->data_length < p_apdu->Lc) {
        NRF_LOG_DEBUG("INIT frame: received data (%d of %d) \r\n", p_apdu->data_length, p_apdu->Lc);
    } else {
        NRF_LOG_DEBUG("INIT frame: received data (%dbytes) \r\n", p_apdu->data_length);
    }
}


static uint16_t get_apdu_le_value(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length)
{
    // Leのバイト数を求める
    uint16_t data_length = control_point_buffer_length - 1;
    uint16_t le_length = (p_apdu->data_length + data_length) - p_apdu->Lc;

    if (le_length == 2) {
        // Leバイトが2バイトの場合
        // Extended Length Encoding と扱い、データの長さを
        // control_point_bufferの末尾２バイトから取得
        p_apdu->Le = (uint32_t)(
            (control_point_buffer[control_point_buffer_length-2] << 8 & 0xFF00) +
             control_point_buffer[control_point_buffer_length-1]);
        if (p_apdu->Le == 0) {
            p_apdu->Le = 65536;
        }
        NRF_LOG_DEBUG("CONT frame: Extended Length Encoding: Le(%d) \r\n", p_apdu->Le);

    } else if (le_length == 1) {
        // Leバイトが1バイトの場合
        // Short Encoding と扱い、データの長さを
        // control_point_bufferの最終バイトから取得
        p_apdu->Le = (uint32_t)control_point_buffer[control_point_buffer_length-1];
        if (p_apdu->Le == 0) {
            p_apdu->Le = 256;
        }
        NRF_LOG_DEBUG("CONT frame: Short Encoding: Le(%d) \r\n", p_apdu->Le);

    } else {
        NRF_LOG_DEBUG("CONT frame: Unknown Encoding: Le(%d bytes) \r\n", le_length);
    }
    
    return le_length;
}

void ble_u2f_control_point_apdu_from_following(BLE_HEADER_T *p_ble_header, U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length)
{
    // 受信データの先頭アドレスとデータ長を取得
    uint8_t *data = control_point_buffer + 1;
    uint16_t data_length = control_point_buffer_length - 1;

    if (p_apdu->data_length + data_length > p_apdu->Lc) {
        // データの最終パケットだが、データ長をオーバーしている場合、
        // オーバーした部分はLeバイトとして扱い、
        // Leバイトを除いた部分を、データ部として扱う
        uint16_t le_length = get_apdu_le_value(p_apdu, control_point_buffer, control_point_buffer_length);
        data_length = data_length - le_length;
    }

    // コピー済みのデータの直後に取得したデータを連結
    memcpy(p_apdu->data + p_apdu->data_length, data, data_length);
    p_apdu->data_length += data_length;
    NRF_LOG_DEBUG("CONT frame: received data (%dbytes) \r\n", p_apdu->data_length);
}


#endif // NRF_MODULE_ENABLED(BLE_U2F)
