/* 
 * File:   fido_ble_unpairing.c
 * Author: makmorit
 *
 * Created on 2022/12/31, 11:20
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_ble_pairing.h"

bool fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size)
{
    // TODO: 仮の実装です。
    return false;
}

void fido_ble_unpairing_cancel_request(void)
{
    // TODO: 仮の実装です。
}

void fido_ble_unpairing_on_disconnect(void)
{
    // TODO: 仮の実装です。
}

void fido_ble_unpairing_done(bool success, uint16_t peer_id)
{
    // TODO: 仮の実装です。
}

//
// ペアリング情報の全削除処理
//
bool fido_ble_unpairing_erase_bond_data(void (*_response_func)(bool))
{
    return app_ble_pairing_erase_bond_data(_response_func);
}

bool fido_ble_unpairing_erase_bond_data_completed(void const *evt)
{
    // nRF52840アプリケーションに用意した関数のため、
    // nRF5340では何もしません
    //   app_ble_pairing_erase_bond_data 内で
    //   等価の処理が行われます
    return false;
}
