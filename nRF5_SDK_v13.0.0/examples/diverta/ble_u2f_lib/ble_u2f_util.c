#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_util"
#include "nrf_log.h"

// for send ble response
#include "ble_u2f.h"
#include "ble_u2f_status.h"

// for SKEY_WORD_NUM,PKEY_WORD_NUM,KEYH_WORD_NUM
#include "ble_u2f_keypair.h"

// for bsp_board_led_on|off, BSP_BOARD_LED_3
#include "bsp.h"

// レスポンス編集エリア
static uint8_t  data_buffer[2];
static uint32_t data_buffer_length;

void ble_u2f_set_status_word(uint8_t *dest_buffer, uint16_t status_word)
{
    // ステータスワードをビッグエンディアンで格納
    dest_buffer[0] = (status_word >> 8) & 0x00ff;
    dest_buffer[1] = (status_word >> 0) & 0x00ff;
}

void ble_u2f_send_success_response(ble_u2f_context_t *p_u2f_context)
{
    // レスポンスを生成
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    ble_u2f_set_status_word(data_buffer, U2F_SW_NO_ERROR);
    data_buffer_length = 2;

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_error_response(ble_u2f_context_t *p_u2f_context, uint16_t err_status_word)
{    
    // コマンドを格納
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;

    // ステータスワードを格納
    ble_u2f_set_status_word(data_buffer, err_status_word);
    data_buffer_length = 2;
    
    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_command_error_response(ble_u2f_context_t *p_u2f_context, uint8_t err_code)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_ERROR;

    // エラーコードを格納
    data_buffer[0]     = err_code;
    data_buffer_length = 1;

    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_keepalive_response(ble_u2f_context_t *p_u2f_context)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_KEEPALIVE;

    // エラーコードを格納
    data_buffer[0]     = p_u2f_context->keepalive_status_byte;
    data_buffer_length = 1;

    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}


uint16_t ble_u2f_copy_keyhandle_data(uint8_t *p_dest_buffer, uint32_t *keypair_cert_buffer)
{
    // キーハンドルは、ワード単位でビッグエンディアン格納されているので
    // バイト単位のアドレスに変換
    uint8_t *p_keyhandle  = (uint8_t *)(keypair_cert_buffer + SKEY_WORD_NUM + PKEY_WORD_NUM);
    uint8_t  keyhandle_length = KEYH_WORD_NUM * sizeof(uint32_t);

    // キーハンドルデータを指定の領域に格納
    memcpy(p_dest_buffer, p_keyhandle, keyhandle_length);
    
    // コピーしたサイズを戻す
    return keyhandle_length;
}


uint16_t ble_u2f_copy_publickey_data(uint8_t *p_dest_buffer, uint32_t *keypair_cert_buffer)
{
    uint16_t copied_size = 0;
    
    // 公開鍵は、ワード単位でリトルエンディアン格納されているので
    // ビッグエンディアンに変換してから格納
    uint8_t *p_publickey  = (uint8_t *)(keypair_cert_buffer + SKEY_WORD_NUM);

    // 1バイト目＝0x04
    p_dest_buffer[copied_size++] = U2F_POINT_UNCOMPRESSED;

    // 2-33バイト目＝公開鍵のx部を設定
    for (uint8_t i = 32; i > 0; i--) {
        p_dest_buffer[copied_size++] = *(p_publickey + i - 1);
    }

    // 34-65バイト目＝公開鍵のy部を設定
    for (uint8_t i = 64; i > 32; i--) {
        p_dest_buffer[copied_size++] = *(p_publickey + i - 1);
    }
        
    // コピーしたサイズを戻す
    return copied_size;
}


// 署名対象、レスポンスデータを格納するバッファ長
#define SIGNATURE_BASE_BUFFER_LENGTH 162
#define RESPONSE_DATA_BUFFER_LENGTH 1024

bool ble_u2f_signature_data_allocate(ble_u2f_context_t *p_u2f_context)
{
    // 署名ベースおよび署名を編集するための
    // ワークエリアをヒープに確保する
    uint8_t *signature_data_buffer = p_u2f_context->signature_data_buffer;
    if (signature_data_buffer != NULL) {
        // 既に確保済みの場合
        NRF_LOG_DEBUG("signature_data_buffer already allocated (%d bytes) \r\n", 
            p_u2f_context->signature_data_buffer_length);
        memset(signature_data_buffer, 0, SIGNATURE_BASE_BUFFER_LENGTH);
        return true;
    }

    // データ格納領域を、データ全体の長さ分確保
    signature_data_buffer = (uint8_t *)malloc(SIGNATURE_BASE_BUFFER_LENGTH);
    if (signature_data_buffer == NULL) {
        NRF_LOG_ERROR("signature_data_buffer allocation failed \r\n");
        return false;
    }

    // 処理内部で確保したヒープの参照先と確保バイト数を保持
    // (Disconnect時に解放されます)
    p_u2f_context->signature_data_buffer        = signature_data_buffer;
    p_u2f_context->signature_data_buffer_length = SIGNATURE_BASE_BUFFER_LENGTH;
    NRF_LOG_DEBUG("signature_data_buffer allocated (%d bytes) \r\n", SIGNATURE_BASE_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(signature_data_buffer, 0, SIGNATURE_BASE_BUFFER_LENGTH);
    return true;
}


bool ble_u2f_response_message_allocate(ble_u2f_context_t *p_u2f_context)
{
    // Register/Authenticationメッセージを
    // 編集するためのワークエリアをヒープに確保する
    uint8_t *response_message_buffer = p_u2f_context->response_message_buffer;
    if (response_message_buffer != NULL) {
        // 既に確保済みの場合
        NRF_LOG_DEBUG("response_message_buffer already allocated (%d bytes) \r\n", 
            p_u2f_context->signature_data_buffer_length);
        memset(response_message_buffer, 0, RESPONSE_DATA_BUFFER_LENGTH);
        return true;
    }

    // データ格納領域を、データ全体の長さ分確保
    response_message_buffer = (uint8_t *)malloc(RESPONSE_DATA_BUFFER_LENGTH);
    if (response_message_buffer == NULL) {
        NRF_LOG_ERROR("response_message_buffer allocation failed \r\n");
        return false;
    }

    // 処理内部で確保したヒープの参照先と確保バイト数を保持
    // (Disconnect時に解放されます)
    p_u2f_context->response_message_buffer        = response_message_buffer;
    p_u2f_context->response_message_buffer_length = RESPONSE_DATA_BUFFER_LENGTH;
    NRF_LOG_DEBUG("response_message_buffer allocated (%d bytes) \r\n", RESPONSE_DATA_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(response_message_buffer, 0, RESPONSE_DATA_BUFFER_LENGTH);
    return true;
}


void ble_u2f_led_light_LED(uint32_t pin_number, bool led_on)
{
    // LEDを出力設定
    nrf_gpio_cfg_output(pin_number);
    if (led_on == true) {
        // LEDを点灯させる
        nrf_gpio_pin_set(pin_number);
    } else {
        // LEDを消灯させる
        nrf_gpio_pin_clear(pin_number);
    }
}


void dump_octets(uint8_t * data, uint16_t length)
{
    char buf[32];
    for (int i = 0; i < length; i+=8) {
        sprintf(buf, " %02x %02x %02x %02x %02x %02x %02x %02x", 
                data[i+0], data[i+1], data[i+2], data[i+3],
                data[i+4], data[i+5], data[i+6], data[i+7]
        );
        NRF_LOG_DEBUG("%s\r\n", (uint32_t)buf);
    }
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
