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

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path);
unsigned char   *tool_crypto_certificate_TLV_data(void);
size_t           tool_crypto_certificate_TLV_size(void);

#endif /* tool_crypto_certificate_h */
