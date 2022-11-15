/* 
 * File:   ccid_flash_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/11/15, 17:44
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "fido_flash_common.h"

#include "ccid_flash_object.h"
#include "ccid_oath.h"
#include "ccid_oath_object.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_oath_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// オブジェクトのRead／Write
//
bool ccid_flash_oath_object_write(uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size, bool use_serial, uint16_t serial)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_find(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_delete(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_delete_all(void)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_fetch(uint16_t obj_tag, int (*_fetch_func)(const char *key, void *data, size_t size))
{
    // TODO: 仮の実装です。
    return false;
}

//
// コールバック関数群
//
void ccid_flash_oath_object_failed(void)
{
    // TODO: 仮の実装です。
}

void ccid_flash_oath_object_gc_done(void)
{
    // TODO: 仮の実装です。
}

void ccid_flash_oath_object_record_updated(void)
{
    // TODO: 仮の実装です。
}

void ccid_flash_oath_object_record_deleted(void)
{
    // TODO: 仮の実装です。
}
