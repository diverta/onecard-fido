/* 
 * File:   sha1.h
 * Author: makmorit
 *
 * Created on 2022/11/24, 18:37
 */
#ifndef SHA1_H
#define SHA1_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void sha1_vector(size_t num_elem, uint8_t *addr[], size_t *len, uint8_t *mac);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_H */
