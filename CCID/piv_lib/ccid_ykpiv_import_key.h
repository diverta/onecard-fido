/* 
 * File:   ccid_ykpiv_import_key.h
 * Author: makmorit
 *
 * Created on 2020/09/14, 11:53
 */
#ifndef CCID_YKPIV_IMPORT_KEY_H
#define CCID_YKPIV_IMPORT_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t ccid_ykpiv_import_key(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_YKPIV_IMPORT_KEY_H */
