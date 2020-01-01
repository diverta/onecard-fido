//
//  usb_cdc_util.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/01.
//
#ifndef usb_cdc_util_h
#define usb_cdc_util_h

#include <stdbool.h>

bool usb_cdc_open_acm_device(const char *path);
void usb_cdc_close_acm_device(void);

#endif /* usb_cdc_util_h */
