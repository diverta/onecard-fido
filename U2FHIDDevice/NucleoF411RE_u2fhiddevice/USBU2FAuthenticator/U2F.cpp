#include <stdio.h>
#include <string.h>

#include "U2F.h"

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
} U2F_HID_INIT_RES;

typedef struct u2f_version_response
{
    uint8_t  version[6];
    uint8_t  status_word[2];
} U2F_VERSION_RES;

U2F_HID_INIT_RES  init_res;
U2F_VERSION_RES   version_res;


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


void generate_hid_init_response(void)
{
    // レスポンスデータを編集 (17 bytes)
    memcpy(init_res.nonce, u2f_request_buffer, 8);
    set_CID(init_res.cid, 0x00003301);
    init_res.version_id    = 2;
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 1;
    
    // レスポンスを格納
    u2f_response_length = sizeof(U2F_HID_INIT_RES);
    memcpy(u2f_response_buffer, &init_res, u2f_response_length);
}

void generate_u2f_version_response(void)
{
    // レスポンスデータを編集 (8 bytes)
    strcpy((char *)version_res.version, "U2F_V2");
    uint16_t status_word = U2F_SW_NO_ERROR;
    version_res.status_word[0] = (status_word >> 8) & 0x00ff;
    version_res.status_word[1] = status_word & 0x00ff;
    
    // レスポンスを格納
    u2f_response_length = sizeof(U2F_VERSION_RES);
    memcpy(u2f_response_buffer, &version_res, u2f_response_length);
}

void generate_u2f_register_response(void)
{
    // U2F管理ツールから転送されたレスポンスデータを設定
    u2f_response_length = u2f_request_length;
    memcpy(u2f_response_buffer, u2f_request_buffer, u2f_request_length);
}

