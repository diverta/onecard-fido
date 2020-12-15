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

typedef struct _cert_desc {
    char *alg_name;
    char  not_after[32];
} CERT_DESC;

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path, unsigned char *cert_data, size_t *cert_size);
bool             tool_crypto_certificate_extract_descriptions(unsigned char *cert_data, size_t cert_size);
CERT_DESC       *tool_crypto_certificate_extracted_descriptions(void);

#endif /* tool_crypto_certificate_h */
