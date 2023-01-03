/* 
 * File:   ccid_openpgp_pin_define.h
 * Author: makmorit
 *
 * Created on 2023/01/03, 14:23
 */
#ifndef CCID_OPENPGP_PIN_DEFINE_H
#define CCID_OPENPGP_PIN_DEFINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// PIN type
typedef enum {
    PIN_TYPE_NONE,
    OPGP_PIN_PW1,
    OPGP_PIN_PW3,
    OPGP_PIN_RC,
} PIN_TYPE;

// PIN object
typedef struct {
    PIN_TYPE    type;
    uint8_t     size_min;
    uint8_t     size_max;
    bool        is_validated;
    uint8_t     current_retries;
    uint8_t     default_retries;
    char       *default_code;
} PIN_T;

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_PIN_DEFINE_H */
