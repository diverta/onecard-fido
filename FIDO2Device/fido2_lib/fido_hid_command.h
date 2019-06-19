/* 
 * File:   fido_hid_command.h
 * Author: makmorit
 *
 * Created on 2018/12/17, 15:11
 */

#ifndef FIDO_HID_COMMAND_H
#define FIDO_HID_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

// for Flash ROM event
#include "fido_flash_event.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// INITコマンドのレスポンスデータ編集領域
//   固定長（17バイト）
//   U2FHID_INIT、CTAPHID_INITで利用
//
typedef struct {
    uint8_t nonce[8];
    uint8_t cid[4];
    uint8_t version_id;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_build;
    uint8_t cflags;
} HID_INIT_RES_T;

void hid_fido_command_send_status_response(uint8_t cmd, uint8_t status_code);
void hid_fido_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number);
void hid_fido_command_on_fs_evt(fido_flash_event_t const *const p_evt);
void hid_fido_command_on_report_started(void);
void hid_fido_command_on_report_completed(void);
void hid_fido_command_set_abort_flag(bool flag);
bool hid_fido_command_is_valid(uint8_t command);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_COMMAND_H */

