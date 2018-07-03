#ifndef U2FHID_H_
#define U2FHID_H_

#include "USBU2FAuthenticator.h"
#include "U2F.h"

extern USBU2FAuthenticator u2fAuthenticator;

//
// HID送信／受信パケット格納領域
//
extern HID_REPORT send_report;
extern HID_REPORT recv_report;

//
// 現在処理中のチャネルID、コマンドを保持
//
extern uint8_t  CMD;
extern uint32_t CID;

//
// 関数群
//
size_t get_payload_length(U2F_HID_MSG *recv_msg);
void   dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain);
void   dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain);

bool   receive_request_data(void);
bool   send_response_packet(void);

#endif // U2FHID_H_
