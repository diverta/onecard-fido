/* 
 * File:   hid_u2f_common.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include "hid_u2f_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// U2Fリクエスト／レスポンスデータ格納領域
// （コマンド共通）
//
uint8_t u2f_request_buffer[1024];
size_t  u2f_request_length;

uint8_t u2f_response_buffer[1024];
size_t  u2f_response_length;

//
// コマンド別のレスポンスデータ編集領域
//   固定長（64バイト）
//
typedef struct u2f_hid_init_response
{
    uint8_t nonce[8];
    uint8_t cid[4];
    uint8_t version_id;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_build;
    uint8_t cflags;
    uint8_t filler[47];
} U2F_HID_INIT_RES;

typedef struct u2f_version_response
{
    uint8_t  version[6];
    uint8_t  status_word[2];
    uint8_t filler[56];
} U2F_VERSION_RES;

U2F_HID_INIT_RES  init_res;
U2F_VERSION_RES   version_res;

//
// 現在処理中のチャネルID、コマンドを保持
//
uint8_t  CMD_for_session;
uint32_t CID_for_session;

// HID INITコマンドで新規発行するHIDを保持
static uint32_t CID_for_initial;

void init_CID(void)
{
    CID_for_initial = U2FHID_INITIAL_CID;
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


void dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain)
{
    size_t len = get_payload_length(recv_msg);
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, CMD: 0x%02x, Payload(%3d bytes)",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.init.cmd, len);

    //size_t cnt = (remain < U2FHID_INIT_PAYLOAD_SIZE) ? remain : U2FHID_INIT_PAYLOAD_SIZE;
    //NRF_LOG_HEXDUMP_DEBUG(recv_msg->pkt.init.payload, cnt);
}

void dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain)
{
    NRF_LOG_DEBUG("%s(%3d bytes) CID: 0x%08x, SEQ: 0x%02x",
        msg_header, size, get_CID(recv_msg->cid), recv_msg->pkt.cont.seq);

    //size_t cnt = (remain < U2FHID_CONT_PAYLOAD_SIZE) ? remain : U2FHID_CONT_PAYLOAD_SIZE;
    //NRF_LOG_HEXDUMP_DEBUG(recv_msg->pkt.cont.payload, cnt);
}


void generate_hid_init_response(void)
{
    // 編集領域を初期化
    memset(&init_res, 0x00, sizeof(init_res));

    // レスポンスデータを編集 (17 bytes)
    //   CIDはインクリメントされたものを設定
    u2f_response_length = 17;
    memcpy(init_res.nonce, u2f_request_buffer, 8);
    set_CID(init_res.cid, ++CID_for_initial);
    init_res.version_id    = 2;
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 0;
    
    // レスポンスを格納
    memcpy(u2f_response_buffer, &init_res, u2f_response_length);
}

void generate_u2f_version_response(void)
{
    // 編集領域を初期化
    memset(&version_res, 0x00, sizeof(version_res));

    // レスポンスデータを編集 (8 bytes)
    u2f_response_length = 8;
    strcpy((char *)version_res.version, "U2F_V2");
    uint16_t status_word = U2F_SW_NO_ERROR;
    version_res.status_word[0] = (status_word >> 8) & 0x00ff;
    version_res.status_word[1] = status_word & 0x00ff;
    
    // レスポンスを格納
    memcpy(u2f_response_buffer, &version_res, u2f_response_length);
}

void generate_u2f_register_response(void)
{
    // U2F Helperから転送されたレスポンスデータを設定
    u2f_response_length = u2f_request_length;
    memcpy(u2f_response_buffer, u2f_request_buffer, u2f_request_length);
}
