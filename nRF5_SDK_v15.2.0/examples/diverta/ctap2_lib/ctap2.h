/* 
 * File:   ctap2.h
 * Author: makmorit
 *
 * Created on 2018/12/17, 10:20
 */
#ifndef CTAP2_H
#define CTAP2_H

#ifdef __cplusplus
extern "C" {
#endif
    
//
// CTAP2をサポートする場合
// trueを設定
//
#define CTAP2_SUPPORTED true

// CTAP2コマンドの識別用
#define CTAP2_COMMAND_PING      0x81
#define CTAP2_COMMAND_INIT      0x86
#define CTAP2_COMMAND_CBOR      0x90
#define CTAP2_COMMAND_ERROR     0xbf

// CTAP2コマンドバイトの識別用
#define CTAP2_CMD_MAKE_CREDENTIAL       0x01
#define CTAP2_CMD_GET_ASSERTION         0x02
#define CTAP2_CMD_GETINFO               0x04
#define CTAP2_CMD_CLIENT_PIN            0x06
#define CTAP2_CMD_GET_NEXT_ASSERTION    0x08

// CTAPHID_INITのオプション識別用
#define CTAP2_CAPABILITY_CBOR   0x04
#define CTAP2_CAPABILITY_NMSG   0x08

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_H */

