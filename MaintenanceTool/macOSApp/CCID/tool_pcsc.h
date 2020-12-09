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

#endif /* tool_pcsc_h */
