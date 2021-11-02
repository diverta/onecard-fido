//
//  mcumgr_cbor_decode.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/02.
//
#ifndef mcumgr_cbor_decode_h
#define mcumgr_cbor_decode_h

bool        mcumgr_cbor_decode_slot_info(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t    *mcumgr_cbor_decode_slot_info_hash(int slot_no);
bool        mcumgr_cbor_decode_slot_info_active(int slot_no);

#endif /* mcumgr_cbor_decode_h */
