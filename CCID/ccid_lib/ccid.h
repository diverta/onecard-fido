/* 
 * File:   ccid.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 12:37
 */
#ifndef CCID_H
#define CCID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void      ccid_initialize_value(void);
bool      ccid_data_frame_received(uint8_t *data, size_t len);
void      ccid_request_apdu_received(void);
uint8_t  *ccid_command_apdu_data(void);
size_t    ccid_command_apdu_size(void);
uint8_t  *ccid_response_apdu_data(void);
void      ccid_response_apdu_size_set(size_t size);
size_t    ccid_response_apdu_size_max(void);
void      ccid_resume_reader_to_pc_data_block(void);
void      ccid_response_time_extension(void);

//
// 共通関数
//
uint16_t  ccid_get_tlv_element_size(uint8_t elem_no, uint8_t *data, size_t size, size_t *elem_header_size, uint16_t *elem_data_size, uint16_t elem_data_size_max);

#ifdef __cplusplus
}
#endif

#endif /* CCID_H */
