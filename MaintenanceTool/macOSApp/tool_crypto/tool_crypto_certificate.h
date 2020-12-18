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

#define CERT_NOT_AFTER_MAX_SIZE 32
#define CERT_SUBJ_NAME_MAX_SIZE 256
#define CERT_HASH_MAX_SIZE      32

typedef struct _cert_desc {
    char         *alg_name;
    char          not_after[CERT_NOT_AFTER_MAX_SIZE];
    char          subject[CERT_SUBJ_NAME_MAX_SIZE];
    char          issuer[CERT_SUBJ_NAME_MAX_SIZE];
    unsigned char hash[CERT_HASH_MAX_SIZE];
} CERT_DESC;

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path, unsigned char *cert_data, size_t *cert_size);
bool             tool_crypto_certificate_extract_descriptions(unsigned char *cert_data, size_t cert_size);
CERT_DESC       *tool_crypto_certificate_extracted_descriptions(void);

#endif /* tool_crypto_certificate_h */
