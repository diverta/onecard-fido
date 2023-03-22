//
//  oath_util.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/16.
//
#ifndef oath_util_h
#define oath_util_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint8_t    *generated_oath_apdu_bytes(void);
size_t      generated_oath_apdu_size(void);
bool        generate_account_add_apdu(const char *account, size_t account_size, const char *base32_secret, size_t base32_secret_size);
bool        generate_apdu_for_calculate(const char *account, size_t account_size);

#endif /* oath_util_h */
