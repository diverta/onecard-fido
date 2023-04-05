//
//  triple_des.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#ifndef triple_des_h
#define triple_des_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool triple_des_import_key(const unsigned char *key_raw, const size_t key_raw_size);
bool triple_des_decrypt(const unsigned char *input, const size_t input_size, unsigned char *output, size_t *output_size);

#endif /* triple_des_h */
