//
//  usb_dfu_util.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/07.
//
#include "nrf52_app_image.h"
#include "usb_dfu_util.h"
#include "debug_log.h"

#define REQUEST_BUFF_SIZE   2560
#define REQUEST_FRAME_SIZE  64

static uint8_t privateRequestBuf[REQUEST_BUFF_SIZE];
static size_t  privateRequestLen;

static uint8_t *privateObjectBuf;
static uint32_t privateObjectLen;
static uint32_t privateObjectIx;
static uint32_t privateObjectCrc;

// MTUを保持（デフォルト＝64バイト）
static uint32_t MTU = REQUEST_FRAME_SIZE;

uint8_t *usb_dfu_object_frame_data(void)
{
    // 送信フレーム格納領域の先頭を戻す
    return privateRequestBuf;
}

size_t usb_dfu_object_frame_size(void)
{
    // 送信フレームのサイズを戻す
    return privateRequestLen;
}

uint32_t usb_dfu_object_checksum_get(void)
{
    // CRC（内部保持値を反転したもの）を戻す
    return ~privateObjectCrc;
}

void usb_dfu_object_checksum_reset(void)
{
    // CRCを初期設定
    privateObjectCrc = 0xffffffff;
}

size_t usb_dfu_object_set_mtu(size_t size)
{
    // MTUは４の倍数とする
    size_t s = (size / 4) * 4;
    MTU = (uint32_t)s;
    return s;
}

static void update_crc(uint8_t b)
{
    uint8_t i;
    uint32_t crc = privateObjectCrc;
    crc ^= b;
    for (i = 0; i < 8; i++) {
        uint32_t m = (crc & 1) ? 0xffffffff : 0;
        crc = (crc >> 1) ^ (0xedb88320u & m);
    }
    privateObjectCrc = crc;
}

static void create_object_frame_to_write(uint8_t requestCode)
{
    // フレームサイズを計算
    uint16_t remaining = privateObjectLen - privateObjectIx;
    if (remaining > MTU) {
        remaining = MTU;
    }

    // 先頭にコマンド種別をセット
    uint8_t *p = &privateRequestBuf[0];
    *p++ = requestCode;
    privateRequestLen = 1;

    // 送信データを設定
    for (size_t i = 0; i < remaining; i++) {
        uint8_t b = privateObjectBuf[privateObjectIx];
        if (b == 0xC0 || b == 0xDB) {
            // C0->DBDC, DB->DBDD に変換
            *p++ = 0xDB;
            *p++ = (b == 0xC0) ? 0xDC : 0xDD;
            privateRequestLen += 2;
        } else {
            *p++ = b;
            privateRequestLen++;
        }
        update_crc(b);
        privateObjectIx++;
    }

    // 終端バイトをセット
    *p = NRF_DFU_BYTE_EOM;
    privateRequestLen++;
}

void usb_dfu_object_frame_init(uint8_t *object_data, size_t object_size)
{
    privateObjectBuf = object_data;
    privateObjectLen = (uint32_t)object_size;
    privateObjectIx = 0;
}

bool usb_dfu_object_frame_prepare(void)
{
    if (privateObjectIx < privateObjectLen) {
        // 送信フレーム格納領域にバイトデータを設定
        create_object_frame_to_write(NRF_DFU_OP_OBJECT_WRITE);
        log_debug("%s: %d bytes prepared", __func__, privateObjectIx);
        return true;

    } else {
        // これ以上送信すべきデータがない場合はfalse
        log_debug("%s: %d bytes prepared, CRC=0x%08x",
                  __func__, privateObjectIx, ~privateObjectCrc);
        return false;
    }
}
