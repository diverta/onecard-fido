/* 
 * File:   ccid_oath_define.h
 * Author: makmorit
 *
 * Created on 2022/05/02, 16:19
 */
#ifndef CCID_OATH_DEFINE_H
#define CCID_OATH_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define OATH_INS_PUT                0x01
#define OATH_INS_DELETE             0x02
#define OATH_INS_CALCULATE          0x04
#define OATH_INS_SELECT             0xA4
#define OATH_INS_RESET              0xAF

#define OATH_TAG_NAME               0x71
#define OATH_TAG_NAME_LIST          0x72
#define OATH_TAG_KEY                0x73
#define OATH_TAG_CHALLENGE          0x74
#define OATH_TAG_META               0x75
#define OATH_TAG_RESPONSE           0x76
#define OATH_TAG_NO_RESP            0x77
#define OATH_TAG_PROPERTY           0x78
#define OATH_TAG_COUNTER            0x7A
#define OATH_TAG_REQ_TOUCH          0x7C
#define OATH_TAG_NEXT_IDX           0x7D

#define OATH_ALG_MASK               0x0F
#define OATH_ALG_SHA1               0x01
#define OATH_ALG_SHA256             0x02

#define OATH_TYPE_MASK              0xF0
#define OATH_TYPE_HOTP              0x10
#define OATH_TYPE_TOTP              0x20

#define OATH_PROP_INC               0x01
#define OATH_PROP_TOUCH             0x02
#define OATH_PROP_EXPORTABLE        0x04
#define OATH_PROP_ALL_FLAGS         0x07

#define MAX_NAME_LEN                64
#define MAX_KEY_LEN                 66
#define MAX_CHALLENGE_LEN           8

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_DEFINE_H */
