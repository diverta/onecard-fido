/* 
 * File:   fido_status.c
 * Author: makmorit
 *
 * Created on 2023/02/07, 12:34
 */
#include "app_process.h"

void fido_status_set_to_idle(void)
{
    // BLEペリフェラルモード時の処理抑止を解除
    app_process_set_pairing_process_flag(false);
}

void fido_status_set_to_busy(void)
{
    // BLEペリフェラルモード時の処理を抑止
    app_process_set_pairing_process_flag(true);
}
