/* 
 * File:   ccid_flash_piv_object.c
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#include "ccid_flash_piv_object.h"

bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg)
{
    return false;
}

bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t *key_size, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_private_key_write(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t key_size)
{
    return false;
}

bool ccid_flash_piv_object_pin_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    return false;
}

bool ccid_flash_piv_object_data_read(uint8_t obj_tag, uint8_t *obj_data, size_t *obj_size, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_data_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    return false;
}

bool ccid_flash_piv_object_data_erase(void)
{
    return false;
}

void ccid_flash_piv_object_failed(void)
{
}

void ccid_flash_piv_object_gc_done(void)
{
}

void ccid_flash_piv_object_record_updated(void)
{
}

void ccid_flash_piv_object_record_deleted(void)
{
}
