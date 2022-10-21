//
//  usb_dfu_util.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/07.
//
#ifndef usb_dfu_util_h
#define usb_dfu_util_h

#include <stdlib.h>

uint8_t *usb_dfu_object_frame_data(void);
size_t   usb_dfu_object_frame_size(void);
uint32_t usb_dfu_object_checksum_get(void);
void     usb_dfu_object_checksum_reset(void);
size_t   usb_dfu_object_set_mtu(size_t size);
void     usb_dfu_object_frame_init(uint8_t *object_data, size_t object_size);
bool     usb_dfu_object_frame_prepare(void);

#endif /* usb_dfu_util_h */
