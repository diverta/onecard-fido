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

// CTAP2コマンドの識別用
#define CTAP2_COMMAND_CBOR      0x90

// CTAP2コマンドオプションの識別用
#define CTAP2_INS_GETINFO       0x04

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_H */

