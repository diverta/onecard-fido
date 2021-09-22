/* 
 * File:   fido_hid_channel.c
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#include <stdio.h>
#include <stdbool.h>
#include "fido_hid_channel.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_hid_channel);
#endif

// HID INITコマンドで新規発行するHIDを保持
static uint32_t CID_for_initial;

// ロック対象CIDを保持
static uint32_t cid_for_lock;

void fido_hid_channel_initialize_cid(void)
{
    CID_for_initial = USBD_HID_INITIAL_CID;
}

uint32_t fido_hid_channel_current_cid(void)
{
    return CID_for_initial;
}

uint32_t fido_hid_channel_new_cid(void)
{
    return ++CID_for_initial;
}

uint32_t fido_hid_channel_get_cid_from_bytes(uint8_t *cid)
{
    uint32_t _CID;

    _CID  = (cid[0] << 24) & 0xff000000;
    _CID |= (cid[1] << 16) & 0x00ff0000;
    _CID |= (cid[2] <<  8) & 0x0000ff00;
    _CID |= (cid[3] <<  0) & 0x000000ff;
    
    return _CID;
}

void fido_hid_channel_set_cid_bytes(uint8_t *cid, uint32_t _CID)
{
    cid[0] = (_CID >> 24) & 0x000000ff;
    cid[1] = (_CID >> 16) & 0x000000ff;
    cid[2] = (_CID >>  8) & 0x000000ff;
    cid[3] = (_CID >>  0) & 0x000000ff;
}

//
// チャネルロック管理
// 
void fido_hid_channel_lock_timedout_handler(void)
{
    // 所定の秒数を経過した場合、
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    fido_log_info("Lock timed out");
}

void fido_hid_channel_lock_start(uint32_t cid, uint8_t lock_param)
{
    // ロックタイマーは最大10秒とする
    if (lock_param > 10) {
        lock_param = 10;
    }
    
    // ロックタイマーを開始
    uint32_t lock_ms = (uint32_t)lock_param * 1000;
    fido_hid_channel_lock_timer_start(lock_ms);

    // パラメーターが指定されていた場合
    // ロック対象CIDを設定
    cid_for_lock = cid;
    fido_log_info("Lock command done: CID(0x%08x) parameter(%d) ", cid, lock_param);
}

uint32_t fido_hid_channel_lock_cid(void)
{
    // 現在ロック対象となっているCIDを戻す
    return cid_for_lock;
}

void fido_hid_channel_lock_cancel(void)
{
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    fido_log_info("Unlock command done ");

    // ロックタイマーを停止
    fido_hid_channel_lock_timer_stop();
}

size_t fido_hid_payload_length_get(USB_HID_MSG_T *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}
