//
//  OpenSSL.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/21.
//

#ifndef OpenSSL_h
#define OpenSSL_h

#include <stdio.h>
#include <stdbool.h>

const char *get_openssl_message(void);
size_t      get_openssl_message_length(void);
void        init_openssl(void);

bool create_keypair_pem_file(const char *output_file_path);
bool create_certreq_csr_file(const char *output_file_path, const char *privkey_file_path,
                             const char *CN, const char *OU, const char *O,
                             const char *L, const char *ST, const char *C);
bool create_selfcrt_crt_file(const char *output_file_path);

#endif /* OpenSSL_h */
