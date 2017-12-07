#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ble_u2f_flash.h"
#include "ble_u2f_securekey.h"
#include "ble_u2f_crypto.h"
#include "ble_u2f_util.h"

// for ble_u2f_crypto_ecb_init
#include "ble_u2f_crypto_ecb.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_securekey"
#include "nrf_log.h"


void ble_u2f_securekey_erase(ble_u2f_context_t *p_u2f_context)
{
    // 秘密鍵／証明書をFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    NRF_LOG_DEBUG("ble_u2f_securekey_erase start \r\n");
    if (ble_u2f_flash_keydata_delete() == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }
}

void ble_u2f_securekey_erase_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // エラーレスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_error_response(p_u2f_context, 0x02);
        NRF_LOG_ERROR("ble_u2f_securekey_erase abend: FDS EVENT=%d \r\n", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_DEL_FILE) {
        // fds_file_delete完了の場合は、AES秘密鍵の初期化処理を行う
        // (fds_record_update/writeまたはfds_gcが実行される)
        if (ble_u2f_crypto_ecb_init() == false) {
            ble_u2f_send_error_response(p_u2f_context, 0x04);
        }

    } else if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // ここでエラーレスポンスを戻す
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_FDS_GC_DONE);
        NRF_LOG_ERROR("ble_u2f_securekey_erase abend: FDS GC done \r\n");

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // fds_record_update/write完了の場合
        // レスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_success_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_securekey_erase end \r\n");
    }
}


static bool convert_skey_bytes_to_word(uint8_t * data, uint16_t length, uint32_t *keydata_buffer)
{
    char buf[20];
    char *endp;
    unsigned long value;
    int index = 0;

    NRF_LOG_DEBUG("convert_skey_bytes_to_word start \r\n");

    memset(buf, 0, sizeof(buf));

    for (int i = 0; i < length; i+=4) {
        // リクエストデータからHex表現文字列を生成
        // この時、エンディアンを考慮し
        // バイト配列を逆にしておく
        sprintf(buf, "0x%02x%02x%02x%02x", 
            data[i + 3],
            data[i + 2],
            data[i + 1],
            data[i + 0]
        );

        // Hex文字列を数値に変換する
        value = strtoul(buf, &endp, 16);
        if (*endp != 0) { 
            // 変換失敗した場合は処理終了
            NRF_LOG_DEBUG("convert_skey_bytes_to_word: %s conversion failed \r\n", (uint32_t)buf);
            return false;

        } else {
            keydata_buffer[index++] = (uint32_t)value;
            NRF_LOG_DEBUG("Converted to [0x%08x]\r\n", value);
        }
    }
    NRF_LOG_DEBUG("convert_skey_bytes_to_word end \r\n");
    return true;
}

void ble_u2f_securekey_install_skey(ble_u2f_context_t *p_u2f_context)
{
    uint8_t *data = p_u2f_context->p_apdu->data;
    uint16_t length = p_u2f_context->p_apdu->data_length;

    // 元データチェック
    if (data == NULL || length == 0) {
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }

    NRF_LOG_DEBUG("ble_u2f_securekey_install_skey start \r\n");

    // Flash ROMに登録済みのデータがあれば領域に読込
    if (ble_u2f_flash_keydata_read(p_u2f_context) == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x02);
        return;
    }
    uint32_t *securekey_buffer = p_u2f_context->securekey_buffer;

    // リクエストデータのバイト変換を行う
    if (convert_skey_bytes_to_word(data, length, securekey_buffer) == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x03);
        return;
    }

    // 秘密鍵をFlash ROMに格納する
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (ble_u2f_flash_keydata_write(p_u2f_context) == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x04);
    }
}

void ble_u2f_securekey_install_skey_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        ble_u2f_send_error_response(p_u2f_context, 0x05);
        NRF_LOG_ERROR("ble_u2f_securekey_install_skey abend: FDS EVENT=%d \r\n", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // ここでエラーレスポンスを戻す
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_FDS_GC_DONE);
        NRF_LOG_ERROR("ble_u2f_securekey_install_skey abend: FDS GC done \r\n");

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_success_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_securekey_install_skey end \r\n");
    }
}


uint8_t *ble_u2f_securekey_skey(ble_u2f_context_t *p_u2f_context)
{
    // 秘密鍵格納領域の開始アドレスを取得
    uint32_t *skey_buffer = p_u2f_context->securekey_buffer;
    return (uint8_t *)skey_buffer;
}


uint8_t *ble_u2f_securekey_cert(ble_u2f_context_t *p_u2f_context)
{
    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = p_u2f_context->securekey_buffer + SKEY_WORD_NUM + 1;
    return (uint8_t *)cert_buffer;
}


uint32_t ble_u2f_securekey_cert_length(ble_u2f_context_t *p_u2f_context)
{
    // 証明書データ格納領域の長さを取得
    uint32_t *cert_buffer = p_u2f_context->securekey_buffer + SKEY_WORD_NUM;
    uint32_t cert_buffer_length = *cert_buffer;
    return cert_buffer_length;
}


void ble_u2f_securekey_install_cert(ble_u2f_context_t *p_u2f_context)
{
    uint8_t *data = p_u2f_context->p_apdu->data;
    uint16_t length = p_u2f_context->p_apdu->data_length;

    // 元データチェック
    if (data == NULL || length == 0) {
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }

    NRF_LOG_DEBUG("ble_u2f_securekey_install_cert start \r\n");

    // 登録済みのデータがあれば領域に読込
    if (ble_u2f_flash_keydata_read(p_u2f_context) == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x02);
        return;
    }

    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *securekey_buffer = p_u2f_context->securekey_buffer;
    uint32_t *cert_buffer = securekey_buffer + SKEY_WORD_NUM;

    // 証明書データの格納に必要なワード数をチェックする
    uint32_t cert_buffer_length = (length - 1) / 4 + 2;
    if (cert_buffer_length > CERT_WORD_NUM) {
        NRF_LOG_ERROR("cert data words(%d) exceeds max words(%d) \r\n",
            cert_buffer_length, CERT_WORD_NUM);
        ble_u2f_send_error_response(p_u2f_context, 0x03);
        return;
    }
    
    // １ワード目に、証明書の当初バイト数を格納し、
    // ２ワード目以降から、証明書のデータを格納するようにする
    // (エンディアンは変換せずにそのまま格納)
    cert_buffer[0] = (uint32_t)length;
    memcpy(cert_buffer + 1, data, length);

    // 証明書データをFlash ROMへ書込
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (ble_u2f_flash_keydata_write(p_u2f_context) == false) {
        ble_u2f_send_error_response(p_u2f_context, 0x04);
    }
}

void ble_u2f_securekey_install_cert_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        ble_u2f_send_error_response(p_u2f_context, 0x05);
        NRF_LOG_ERROR("ble_u2f_securekey_install_cert abend: FDS EVENT=%d \r\n", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // ここでエラーレスポンスを戻す
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_FDS_GC_DONE);
        NRF_LOG_ERROR("ble_u2f_securekey_install_cert abend: FDS GC done \r\n");

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_success_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_securekey_install_cert end \r\n");
    }
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
