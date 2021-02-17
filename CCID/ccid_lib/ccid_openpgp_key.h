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

// Key type
typedef enum {
    OPGP_KEY_SIG,
    OPGP_KEY_ENC,
    OPGP_KEY_AUT,
} OPGP_KEY_TYPE;  
    
//
// 関数群
//
uint16_t    openpgp_key_get_attributes(uint16_t tag, uint8_t *buf, size_t *size);
uint16_t    openpgp_key_get_fingerprint(uint16_t tag, void *buf, size_t *size);
uint16_t    openpgp_key_get_datetime(uint16_t tag, void *buf, size_t *size);
uint16_t    openpgp_key_get_status(uint16_t tag, uint8_t *status);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_KEY_H */
