//
//  usb_cdc_util.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/01.
//
#ifndef usb_cdc_util_h
#define usb_cdc_util_h

#include <stdbool.h>

bool usb_cdc_acm_device_is_not_busy(const char *path);
bool usb_cdc_acm_device_open(const char *path);
bool usb_cdc_acm_device_write(const char *data, size_t size);
void usb_cdc_acm_device_close(void);

uint8_t *usb_cdc_acm_device_read_buffer(void);
size_t   usb_cdc_acm_device_read_size(void);
bool     usb_cdc_acm_device_read(double read_timeout_sec);

#endif /* usb_cdc_util_h */
