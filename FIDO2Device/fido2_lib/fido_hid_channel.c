/* 
 * File:   fido_hid_channel.c
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#include <stdio.h>
#include <stdbool.h>
#include "fido_hid_channel.h"

#include "fido_timer.h"
#include "fido_log.h"

// HID INITコマンドで新規発行するHIDを保持
static uint32_t CID_for_initial;

// ロック対象CIDを保持
static uint32_t cid_for_lock;

void init_CID(void)
{
    CID_for_initial = USBD_HID_INITIAL_CID;
}

uint32_t get_current_CID(void)
{
    return CID_for_initial;
}

uint32_t get_incremented_CID(void)
{
    return ++CID_for_initial;
}

uint32_t get_CID(uint8_t *cid)
{
    uint32_t _CID;

    _CID  = (cid[0] << 24) & 0xff000000;
    _CID |= (cid[1] << 16) & 0x00ff0000;
    _CID |= (cid[2] <<  8) & 0x0000ff00;
    _CID |= (cid[3] <<  0) & 0x000000ff;
    
    return _CID;
}

void set_CID(uint8_t *cid, uint32_t _CID)
{
    cid[0] = (_CID >> 24) & 0x000000ff;
    cid[1] = (_CID >> 16) & 0x000000ff;
    cid[2] = (_CID >>  8) & 0x000000ff;
    cid[3] = (_CID >>  0) & 0x000000ff;
}

//
// チャネルロック管理
// 
void fido_lock_channel_timedout_handler(void *context)
{
    // 所定の秒数を経過した場合、
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    fido_log_info("Lock timed out");
}

void fido_lock_channel_start(uint32_t cid, uint8_t lock_param)
{
    // ロックタイマーは最大10秒とする
    if (lock_param > 10) {
        lock_param = 10;
    }
    
    // ロックタイマーを開始
    uint32_t lock_ms = (uint32_t)lock_param * 1000;
    fido_lock_channel_timer_start(lock_ms);

    // パラメーターが指定されていた場合
    // ロック対象CIDを設定
    cid_for_lock = cid;
    fido_log_info("Lock command done: CID(0x%08x) parameter(%d) ", cid, lock_param);
}

uint32_t fido_lock_channel_cid(void)
{
    // 現在ロック対象となっているCIDを戻す
    return cid_for_lock;
}

void fido_lock_channel_cancel(void)
{
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    fido_log_info("Unlock command done ");

    // ロックタイマーを停止
    fido_lock_channel_timer_stop();
}

size_t get_payload_length(USB_HID_MSG_T *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}
