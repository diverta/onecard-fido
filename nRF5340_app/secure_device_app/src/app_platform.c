/* 
 * File:   app_platform.c
 * Author: makmorit
 *
 * Created on 2021/08/19, 9:52
 */
#include <zephyr/types.h>
#include <zephyr.h>

//
// トランスポート関連
//
#include "app_usb_hid.h"

void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    app_usb_hid_send_report(buffer_for_send, size);
}

//
// DFU関連
//
#include "app_dfu.h"

void usbd_service_stop_for_bootloader(void)
{
    // ブートローダーモード遷移を指示
    app_dfu_prepare_for_bootloader();
}
