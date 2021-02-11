/* 
 * File:   ccid_pin.h
 * Author: makmorit
 *
 * Created on 2021/02/11, 9:43
 */
#ifndef CCID_PIN_H
#define CCID_PIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// PIN type
typedef enum {
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
    uint8_t     default_retries;
} PIN_T;

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIN_H */
