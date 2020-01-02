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
#include <math.h>
#include <termios.h>
#include <unistd.h>

#include "usb_cdc_util.h"
#include "debug_log.h"

// ファイル記述子を保持
static int file_descriptor;

// シリアルポート属性を保持
static struct termios original_tc_attrs;

// 読込可能バイト
static uint8_t _buffer[4096];
static size_t   bytesRead;

bool usb_cdc_acm_device_open(const char *path)
{
    // シリアルポートを開く
    int flags = (O_RDWR | O_NOCTTY);
    file_descriptor = open(path, flags);
    if (file_descriptor < 0) {
        log_debug("%s: Opening %s failed with error: %s(%d)", __func__,
                  path, strerror(errno), errno);
        return false;
    }

    // シリアルポートの属性を取得
    if (tcgetattr(file_descriptor, &original_tc_attrs) == -1) {
        log_debug("%s: Getting TTYattr %@ failed with error: %s(%d)", __func__,
                  path, strerror(errno), errno);
        close(file_descriptor);
        return false;
    }
    
    return true;
}

bool usb_cdc_acm_device_write(const char *data, size_t size)
{
    if (data == NULL || size == 0) {
        return false;
    }

    size_t size_wrote = write(file_descriptor, data, size);
    if (size_wrote != size) {
        log_debug("%s: write failed (%d bytes wrote, %d bytes to be written)", __func__,
                  size_wrote, size);
        return false;
    }

    log_debug("%s: write success (%d bytes wrote, %d bytes to be written)", __func__,
              size_wrote, size);
    return true;
}

void usb_cdc_acm_device_close(void)
{
    close(file_descriptor);
}

//
// バッファ読込関連処理
//
uint8_t *usb_cdc_acm_device_read_buffer(void)
{
    return _buffer;
}

size_t usb_cdc_acm_device_read_size(void)
{
    return bytesRead;
}

static void set_select_timeout(struct timeval *timeout, uint8_t timeout_sec)
{
    // convert select_timeout --> struct timeval
    int64_t timeout_usec = llround(timeout_sec * 1000000.0);
    timeout->tv_sec = (__darwin_time_t)timeout_sec;
    timeout->tv_usec = (__darwin_suseconds_t)(timeout_usec - timeout_sec * 1000000);
}

bool usb_cdc_acm_device_read(void)
{
    // 読込バッファを初期化
    bytesRead = 0;
    memset(_buffer, 0, sizeof(_buffer));

    // select実行用のファイル記述子を設定
    fd_set read_fd;
    FD_ZERO(&read_fd);
    FD_SET(file_descriptor, &read_fd);

    // select実行のタイムアウトを１秒に設定
    struct timeval timeout;
    set_select_timeout(&timeout, 1);

    // selectを実行し、ポートが読込可能になるまで監視
    int select_result = select(file_descriptor + 1, &read_fd, NULL, NULL, &timeout);
    if (select_result == -1) {
        // エラー発生時
        log_debug("%s: read failed (select() returns %d)", __func__, select_result);
        return false;

    } else if (select_result == 0) {
        log_debug("%s: read timed out (select() returns %d)", __func__, select_result);
        return false;
    }

    // 読込開始
    size_t sizeToRead = sizeof(_buffer);
    ssize_t read_result = read(file_descriptor, _buffer + bytesRead, sizeToRead);
    if (read_result == 0) {
        log_debug("%s: read failed (this should be impossible since select() has indicated data is available)", __func__);
        return false;

    } else if (read_result < 0) {
        log_debug("%s: unknown fatal error (read() returns %d)", __func__, read_result);
        return false;
    }

    // バッファにこれ以上格納できない場合は中断
    bytesRead += read_result;
    if (bytesRead >= sizeof(_buffer)) {
        log_debug("%s: read failed (internal buffer is full)", __func__);
        return false;
    }

    return true;
}
