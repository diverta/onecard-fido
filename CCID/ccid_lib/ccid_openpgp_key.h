/* 
 * File:   ccid_openpgp_key.h
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:20
 */
#ifndef CCID_OPENPGP_KEY_H
#define CCID_OPENPGP_KEY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_FINGERPRINT_LENGTH      20
#define KEY_DATETIME_LENGTH         4

//
// 関数群
//
uint16_t    ccid_openpgp_key_pair_generate(command_apdu_t *capdu, response_apdu_t *rapdu);
uint16_t    ccid_openpgp_key_status_tag_get(uint16_t key_tag);
uint16_t    openpgp_key_get_attributes(uint16_t tag, uint8_t *buf, size_t *size);
uint16_t    openpgp_key_get_fingerprint(uint16_t tag, void *buf, size_t *size);
uint16_t    openpgp_key_get_datetime(uint16_t tag, void *buf, size_t *size);
uint16_t    openpgp_key_get_status(uint16_t key_tag, uint8_t *status);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_KEY_H */
