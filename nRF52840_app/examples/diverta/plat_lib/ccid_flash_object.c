/* 
 * File:   ccid_flash_object.c
 * Author: makmorit
 *
 * Created on 2021/02/15, 11:58
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "ccid_flash_openpgp_object.h"
#include "ccid_flash_piv_object.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// レコード格納領域
//   バッファ長（MAX_BUF_SIZE）は、
//   このモジュールで管理する
//   最大のレコードサイズに合わせます。
#define MAX_BUF_SIZE     (PIV_DATA_OBJ_DATA_WORDS_MAX+1)
static uint32_t          m_record_buf_R[MAX_BUF_SIZE];
static uint32_t          m_record_buf_W[MAX_BUF_SIZE];

uint8_t *ccid_flash_object_read_buffer(void)
{
    return (uint8_t *)m_record_buf_R;
}

uint8_t *ccid_flash_object_write_buffer(void)
{
    return (uint8_t *)m_record_buf_W;
}

size_t ccid_flash_object_rw_buffer_size(void)
{
    return MAX_BUF_SIZE;
}

size_t ccid_flash_object_calculate_words(size_t record_bytes)
{
    // オブジェクトの長さから、必要ワード数を計算
    size_t record_words = record_bytes / 4;
    if (record_bytes % 4 > 0) {
        record_words++;
    }
    return record_words;
}

//
// コールバック関数群
//
void ccid_flash_object_failed(void)
{
    // CCID関連処理を実行
    ccid_flash_piv_object_failed();
    ccid_flash_openpgp_object_failed();
}

void ccid_flash_object_gc_done(void)
{
    // CCID関連処理を実行
    ccid_flash_piv_object_gc_done();
    ccid_flash_openpgp_object_gc_done();
}

void ccid_flash_object_record_updated(void)
{
    // CCID関連処理を実行
    ccid_flash_piv_object_record_updated();
    ccid_flash_openpgp_object_record_updated();
}

void ccid_flash_object_record_deleted(void)
{
    // CCID関連処理を実行
    ccid_flash_piv_object_record_deleted();
    ccid_flash_openpgp_object_record_deleted();
}
