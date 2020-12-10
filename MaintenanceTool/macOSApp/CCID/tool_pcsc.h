//
//  tool_pcsc.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/09.
//
#ifndef tool_pcsc_h
#define tool_pcsc_h

#include <stdbool.h>
#include <stdio.h>

const char      *tool_pcsc_scard_slot_name(void);
void             tool_pcsc_scard_init(void);
bool             tool_pcsc_scard_connect(void);
bool             tool_pcsc_scard_begin_transaction(void);
void             tool_pcsc_scard_end_transaction(void);
void             tool_pcsc_scard_disconnect(void);
void             tool_pcsc_scard_set_command_apdu(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t lc, uint8_t *data, uint16_t le);
bool             tool_pcsc_scard_send_command_apdu(void);
uint8_t         *tool_pcsc_scard_response_apdu_data(size_t *apdu_data_size, uint16_t *sw);

#endif /* tool_pcsc_h */
