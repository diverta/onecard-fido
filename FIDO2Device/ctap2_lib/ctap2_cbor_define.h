/* 
 * File:   ctap2_cbor_define.h
 * Author: makmorit
 *
 * Created on 2021/10/04, 9:35
 */
#ifndef CTAP2_CBOR_DEFINE_H
#define CTAP2_CBOR_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

// Public Key Credential Type
#define PUB_KEY_CRED_PUB_KEY        0x01
#define PUB_KEY_CRED_UNKNOWN        0x3F

// Values for COSE_Key format
//  Key type
#define COSE_KEY_LABEL_KTY          1
#define COSE_KEY_KTY_EC2            2
//  Curve type
#define COSE_KEY_LABEL_CRV          -1
#define COSE_KEY_CRV_P256           1
//  Key coordinate
#define COSE_KEY_LABEL_X            -2
#define COSE_KEY_LABEL_Y            -3
//  Signature algorithm
#define COSE_KEY_LABEL_ALG          3
#define COSE_ALG_ES256              -7

// Credential type suppurted or not
#define CREDENTIAL_IS_SUPPORTED     1
#define CREDENTIAL_NOT_SUPPORTED    0

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CBOR_DEFINE_H */
