#include <stdio.h>

#include "mbed.h"
#include "U2FHID.h"

//
// 現在処理中のチャネルID、コマンドを保持
//
uint8_t  CMD;
uint32_t CID;


size_t get_payload_length(U2F_HID_MSG *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}

void dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain)
{
    printf("%s(%3d bytes) ", msg_header, size);
    printf("CID: 0x%08x, ",  get_CID(recv_msg->cid));
    printf("CMD: 0x%02x, ",  recv_msg->pkt.init.cmd);

    size_t len = get_payload_length(recv_msg);
    printf("Payload(%3d bytes): ", len);
    
    size_t cnt = (remain < U2FHID_INIT_PAYLOAD_SIZE) ? remain : U2FHID_INIT_PAYLOAD_SIZE;
    for(size_t i = 0; i < cnt; i++) {
        printf("%02x ", recv_msg->pkt.init.payload[i]);
    }
    printf("\r\n");
}

void dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain)
{
    printf("%s(%3d bytes) ", msg_header, size);
    printf("CID: 0x%08x, ",  get_CID(recv_msg->cid));
    printf("SEQ: 0x%02x, ",  recv_msg->pkt.cont.seq);
    
    size_t cnt = (remain < U2FHID_CONT_PAYLOAD_SIZE) ? remain : U2FHID_CONT_PAYLOAD_SIZE;
    for(size_t i = 0; i < cnt; i++) {
        printf("%02x ", recv_msg->pkt.cont.payload[i]);
    }
    printf("\r\n");
}
