//
//  nrf52_app_image.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/06.
//
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "debug_log.h"
#include "nrf52_app_image.h"

//
// nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
// .dat=256バイト、.bin=512Kバイトと見積っています。
//
static uint8_t nrf52_app_dat[256];
static uint8_t nrf52_app_bin[524288];
static uint8_t nrf52_app_zip[524288];

static size_t nrf52_app_dat_size;
static size_t nrf52_app_bin_size;
static size_t nrf52_app_zip_size;

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

// ファームウェア更新イメージ（app_dfu_package.<バージョン文字列>.zip）のフルパスを保持
static char nrf52_app_zip_filename[1024];
static char nrf52_app_zip_version[16];
static char nrf52_app_zip_boardname[16];

char *nrf52_app_image_zip_filename(void)
{
    return nrf52_app_zip_filename;
}

char *nrf52_app_image_zip_version(void)
{
    return nrf52_app_zip_version;
}

char *nrf52_app_image_zip_boardname(void)
{
    return nrf52_app_zip_boardname;
}

static bool read_app_image_file(const char *file_name, size_t max_size, uint8_t *data, size_t *size)
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

static uint16_t convert_le_bytes_to_uint16(uint8_t *data, size_t offset) {
    uint8_t *bytes = (uint8_t *)data;
    uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
    return uint;
}

static uint32_t convert_le_bytes_to_uint32(uint8_t *data, size_t offset) {
    uint8_t *bytes = (uint8_t *)data;
    uint32_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8)
    | ((uint32_t)bytes[offset + 2] << 16) | ((uint32_t)bytes[offset + 3] << 24);
    return uint;
}

static size_t parse_file(uint8_t *data)
{
    char file_name[64];
    // ファイルのサイズ
    size_t offset = 18;
    size_t compressed_size = (size_t)convert_le_bytes_to_uint32(data, offset);
    offset += 8;
    // ファイル名のサイズ
    size_t filename_size = (size_t)convert_le_bytes_to_uint16(data, offset);
    offset += 2;
    // コメントのサイズ
    size_t comment_size = (size_t)convert_le_bytes_to_uint16(data, offset);
    offset += 2;
    // ファイル名
    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, (char *)(data + offset), filename_size);
    offset += filename_size;
    // コメント
    offset += comment_size;
    // ファイルの内容
    uint8_t *p_data = data + offset;
    if (strcmp(file_name, NRF52_APP_DAT_FILE_NAME) == 0) {
        // .datファイルのバイナリーイメージを配列に格納
        memcpy(nrf52_app_dat, p_data, compressed_size);
        nrf52_app_dat_size = compressed_size;

    } else if (strcmp(file_name, NRF52_APP_BIN_FILE_NAME) == 0) {
        // .binファイルのバイナリーイメージを配列に格納
        memcpy(nrf52_app_bin, p_data, compressed_size);
        nrf52_app_bin_size = compressed_size;
    }
    // 書庫エントリーのサイズを戻す
    offset += compressed_size;
    return offset;
}

static void extract_fw_version(char *file_name)
{
    // 編集領域を初期化
    memset(nrf52_app_zip_version, 0, sizeof(nrf52_app_zip_version));
    memset(nrf52_app_zip_boardname, 0, sizeof(nrf52_app_zip_boardname));
    // ファイル名をバッファにコピー
    char buf[32];
    strcpy(buf, file_name);
    // ファイル名（appkg.PCA10059.0.2.4.zip）からバージョン情報を抽出して保持
    int ver = 0, rev = 0, sub = 0;
    char *p = strtok(buf, ".");
    if (p) {
        p = strtok(NULL, ".");
        if (p) {
            strncpy(nrf52_app_zip_boardname, p, strlen(p));
            p = strtok(NULL, ".");
            if (p) {
                ver = atoi(p);
                p = strtok(NULL, ".");
                if (p) {
                    rev = atoi(p);
                    p = strtok(NULL, ".");
                    if (p) {
                        sub = atoi(p);
                    }
                }
            }
        }
    }
    sprintf(nrf52_app_zip_version, "%d.%d.%d", ver, rev, sub);
}

bool nrf52_app_image_zip_filename_get(const char *zip_file_dir_path, const char *zip_file_name_prefix)
{
    DIR *dir = opendir(zip_file_dir_path);
    if (dir == NULL) {
        return false;
    }
    bool found = false;
    memset(nrf52_app_zip_filename, 0, sizeof(nrf52_app_zip_filename));
    struct dirent *dp = readdir(dir);
    while (dp != NULL) {
        // ファイル名に zip_file_name_prefix が含まれている場合
        if (strncmp(dp->d_name, zip_file_name_prefix, strlen(zip_file_name_prefix)) == 0) {
            // フルパスを編集して保持
            sprintf(nrf52_app_zip_filename, "%s/%s", zip_file_dir_path, dp->d_name);
            // ファイル名からバージョン番号を抽出して保持
            extract_fw_version(dp->d_name);
            found = true;
            break;
        }
        dp = readdir(dir);
    }
    closedir(dir);
    return found;
}

bool nrf52_app_image_zip_read(const char *zip_file_path)
{
    // データバッファ／サイズを初期化
    memset(nrf52_app_zip, 0, sizeof(nrf52_app_zip));
    nrf52_app_zip_size = 0;
    // .zipファイルを読込
    size_t max_size = sizeof(nrf52_app_zip);
    if (read_app_image_file(zip_file_path, max_size, nrf52_app_zip, &nrf52_app_zip_size) == false) {
        return false;
    }
    // .zip書庫ファイルを解析し、内包されている.bin/.datイメージを抽出
    size_t i = 0;
    while (i < nrf52_app_zip_size) {
        if (nrf52_app_zip[i+0] == 0x50 && nrf52_app_zip[i+1] == 0x4B &&
            nrf52_app_zip[i+2] == 0x03 && nrf52_app_zip[i+3] == 0x04) {
            // 書庫エントリーのヘッダー（50 4B 03 04）が見つかった場合
            i += parse_file(&nrf52_app_zip[i]);
        } else {
            i++;
        }
    }
    return true;
}
