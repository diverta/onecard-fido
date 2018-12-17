/* 
 * File:   usbd_hid_common.c
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#include <stdio.h>
#include "u2f.h"
#include "usbd_hid_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_hid_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// HID INITコマンドで新規発行するHIDを保持
static uint32_t CID_for_initial;

void init_CID(void)
{
    CID_for_initial = USBD_HID_INITIAL_CID;
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

size_t get_payload_length(USB_HID_MSG_T *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}

void dump_hid_init_packet(char *msg_header, size_t size, USB_HID_MSG_T *recv_msg)
{
    size_t len = get_payload_length(recv_msg);
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, CMD: 0x%02x, Payload(%3d bytes)",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.init.cmd, len);
}

void dump_hid_cont_packet(char *msg_header, size_t size, USB_HID_MSG_T *recv_msg)
{
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, SEQ: 0x%02x",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.cont.seq);
}
