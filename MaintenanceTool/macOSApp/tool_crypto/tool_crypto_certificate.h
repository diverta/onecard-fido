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

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path, unsigned char *cert_data, size_t *cert_size);

#endif /* tool_crypto_certificate_h */
