/* 
 * File:   fido_status.c
 * Author: makmorit
 *
 * Created on 2023/02/07, 12:34
 */
#include "ble_service_common.h"

void fido_status_set_to_idle(void)
{
    // BLEペリフェラルモード時の処理抑止を解除
    ble_service_peripheral_set_busy(false);
}

void fido_status_set_to_busy(void)
{
    // BLEペリフェラルモード時の処理を抑止
    ble_service_peripheral_set_busy(true);
}
