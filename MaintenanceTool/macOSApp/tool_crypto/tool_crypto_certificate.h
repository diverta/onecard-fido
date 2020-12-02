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

//
// public functions
//
bool             tool_crypto_certificate_extract_from_pem(const char *pem_path);

#endif /* tool_crypto_certificate_h */
