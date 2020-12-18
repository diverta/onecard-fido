//
//  tool_crypto_common.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/03.
//
#ifndef tool_crypto_common_h
#define tool_crypto_common_h

#define CRYPTO_ALG_NONE             0x00
#define CRYPTO_ALG_3DES             0x03
#define CRYPTO_ALG_RSA2048          0x07
#define CRYPTO_ALG_ECCP256          0x11

#define DES_TYPE_3DES               1
#define DES_LEN_DES                 8
#define DES_LEN_3DES                (DES_LEN_DES*3)

#define RSA2048_N_SIZE              2048
#define RSA2048_PQ_SIZE             128
#define RSA2048_PKEY_TLV_SIZE       655

#define ECCP256_KEY_SIZE            32
#define ECCP256_PKEY_TLV_SIZE       34

#define CERTIFICATE_MAX_SIZE        3072

//
// public functions
//
unsigned char    tool_crypto_get_algorithm_from_evp_pkey(void *_key);

#endif /* tool_crypto_common_h */
