/* 
 * File:   hid_u2f_common.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include "hid_u2f_common.h"
#include "u2f.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug request data
#define NRF_LOG_HEXDUMP_DEBUG_PACKET 0

//
// U2Fレスポンスデータ格納領域
// （コマンド共通）
//
uint8_t u2f_response_buffer[1024];
size_t  u2f_response_length;

// HID INITコマンドで新規発行するHIDを保持
static uint32_t CID_for_initial;

void init_CID(void)
{
    CID_for_initial = U2FHID_INITIAL_CID;
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

size_t get_payload_length(U2F_HID_MSG *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}


void dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg)
{
    size_t len = get_payload_length(recv_msg);
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, CMD: 0x%02x, Payload(%3d bytes)",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.init.cmd, len);
#if NRF_LOG_HEXDUMP_DEBUG_PACKET
    NRF_LOG_HEXDUMP_DEBUG(recv_msg, sizeof(U2F_HID_MSG));
#endif
}

void dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg)
{
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, SEQ: 0x%02x",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.cont.seq);
#if NRF_LOG_HEXDUMP_DEBUG_PACKET
    NRF_LOG_HEXDUMP_DEBUG(recv_msg, sizeof(U2F_HID_MSG));
#endif
}

void generate_u2f_error_response(uint8_t error_code)
{
    // レスポンスデータを編集 (1 bytes)
    u2f_response_length = 1;
    u2f_response_buffer[0] = error_code;
}
