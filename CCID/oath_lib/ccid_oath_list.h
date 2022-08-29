/* 
 * File:   ccid_oath_list.h
 * Author: makmorit
 *
 * Created on 2022/08/25, 16:12
 */
#ifndef CCID_OATH_LIST_H
#define CCID_OATH_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_list(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_LIST_H */
