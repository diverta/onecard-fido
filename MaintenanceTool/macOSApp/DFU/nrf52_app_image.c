//
//  nrf52_app_image.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/06.
//
#include <stdio.h>
#include <string.h>

#include "debug_log.h"
#include "nrf52_app_image.h"

//
// nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
// .dat=256バイト、.bin=512Kバイトと見積っています。
//
static uint8_t nrf52_app_dat[256];
static uint8_t nrf52_app_bin[524288];

static size_t nrf52_app_dat_size;
static size_t nrf52_app_bin_size;

uint8_t *nrf52_app_image_dat(void)
{
    return nrf52_app_dat;
}

uint8_t *nrf52_app_image_bin(void)
{
    return nrf52_app_bin;
}

size_t nrf52_app_image_dat_size(void)
{
    return nrf52_app_dat_size;
}

size_t nrf52_app_image_bin_size(void)
{
    return nrf52_app_bin_size;
}

bool read_app_image_file(const char *file_name, size_t max_size, uint8_t *data, size_t *size)
{
    int c = 0;
    size_t i = 0;

    FILE *f = fopen(file_name, "rb");
    if (f == NULL) {
        log_debug("%s: fopen failed (%s)", __func__, file_name);
        return false;
    }

    while (EOF != (c = fgetc(f))) {
        data[i] = (uint8_t)c;
        if (++i == max_size) {
            // 読み込み可能最大サイズを超えた場合はfalse
            fclose(f);
            log_debug("%s: read size reached max size (%d bytes)", __func__, max_size);
            return false;
        }
    }

    // 読込サイズを設定して戻る
    *size = i;
    fclose(f);
    return true;
}

bool nrf52_app_image_dat_read(const char *dat_file_path)
{
    // データバッファ／サイズを初期化
    memset(nrf52_app_dat, 0, sizeof(nrf52_app_dat));
    nrf52_app_dat_size = 0;

    // .datファイルを読込
    size_t max_size = sizeof(nrf52_app_dat);
    if (read_app_image_file(dat_file_path, max_size, nrf52_app_dat, &nrf52_app_dat_size) == false) {
        return false;
    }

    return true;
}

bool nrf52_app_image_bin_read(const char *bin_file_path)
{
    // データバッファ／サイズを初期化
    memset(nrf52_app_bin, 0, sizeof(nrf52_app_bin));
    nrf52_app_bin_size = 0;

    // .binファイルを読込
    size_t max_size = sizeof(nrf52_app_bin);
    if (read_app_image_file(bin_file_path, max_size, nrf52_app_bin, &nrf52_app_bin_size) == false) {
        return false;
    }
    
    return true;
}
