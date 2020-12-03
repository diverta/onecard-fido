//
//  tool_crypto_certificate.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/02.
//
#ifndef tool_crypto_certificate_h
#define tool_crypto_certificate_h

#include <stdbool.h>
#include <stdio.h>

#define CERTIFICATE_MAX_SIZE        3072

#define TAG_CERT                    0x70
#define TAG_CERT_COMPRESS           0x71
#define TAG_CERT_LRC                0xfe

#define TAG_DATA_OBJECT             0x5c
#define TAG_DATA_OBJECT_VALUE       0x53

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path);
unsigned char   *tool_crypto_certificate_APDU_data(unsigned char key_slot_id);
size_t           tool_crypto_certificate_APDU_size(void);

#endif /* tool_crypto_certificate_h */
