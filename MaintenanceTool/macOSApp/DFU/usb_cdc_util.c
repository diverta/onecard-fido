//
//  usb_cdc_util.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/01.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "usb_cdc_util.h"
#include "debug_log.h"

// ファイル記述子を保持
static int file_descriptor;

// シリアルポート属性を保持
static struct termios *original_tc_attrs;

bool usb_cdc_open_acm_device(const char *path)
{
    // シリアルポートを開く
    int flags = (O_RDWR | O_NOCTTY);
    file_descriptor = open(path, flags);
    if (file_descriptor < 0) {
        log_debug("%s: Opening %s failed with error: %s(%d)", __func__,
                  path, strerror(errno), errno);
        return false;
    }
    
    // シリアルポート属性の格納領域を確保
    original_tc_attrs = (struct termios *)malloc(sizeof(struct termios));
    if (original_tc_attrs == NULL) {
        log_debug("%s: termios allocation failed", __func__);
        close(file_descriptor);
        return false;
    }
    
    // シリアルポートの属性を取得
    if (tcgetattr(file_descriptor, original_tc_attrs) == -1) {
        log_debug("%s: Getting TTYattr %@ failed with error: %s(%d)", __func__,
                  path, strerror(errno), errno);
        close(file_descriptor);
        return false;
    }
    
    return true;
}

void usb_cdc_close_acm_device(void)
{
    close(file_descriptor);
    free(original_tc_attrs);
}
