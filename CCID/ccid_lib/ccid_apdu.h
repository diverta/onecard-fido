/* 
 * File:   ccid_apdu.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#ifndef CCID_APDU_H
#define CCID_APDU_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APDU_BUFFER_SIZE            1280
#define APDU_DATA_SIZE              (APDU_BUFFER_SIZE + 2)

//
// 構造体定義
//
typedef struct command_apdu {
    uint8_t  cla;
    uint8_t  ins;
    uint8_t  p1;
    uint8_t  p2;
    size_t   lc;
    size_t   le;
    uint8_t *data;
} command_apdu_t;

typedef struct response_apdu {
    uint8_t  data[APDU_BUFFER_SIZE];
    uint16_t len;
    uint16_t sw;
    size_t   already_sent;
} response_apdu_t;

//
// 関数群
//
void ccid_apdu_process(void);
void ccid_apdu_resume_process(command_apdu_t *capdu, response_apdu_t *rapdu);
bool ccid_apdu_response_is_pending(void);
void ccid_apdu_response_set_pending(bool b);

#ifdef __cplusplus
}
#endif

#endif /* CCID_APDU_H */
