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
bool        generate_account_add_apdu(const char *account, const char *base32_secret);
bool        generate_apdu_for_calculate(const char *account);

#endif /* oath_util_h */
