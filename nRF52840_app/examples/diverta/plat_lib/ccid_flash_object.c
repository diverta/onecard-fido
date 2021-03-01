/* 
 * File:   ccid_flash_object.c
 * Author: makmorit
 *
 * Created on 2021/02/15, 11:58
 */
#include "sdk_common.h"

#include "fido_flash.h"

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
