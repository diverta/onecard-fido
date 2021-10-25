//
//  mcumgr_app_image.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/20.
//
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "debug_log.h"
#include "mcumgr_app_image.h"

// ファームウェア更新イメージ（app_update.PCA10095.0.4.0.bin）のフルパスを保持
static char mcumgr_app_bin_filename[1024];
static char mcumgr_app_bin_version[16];
static char mcumgr_app_bin_boardname[16];

char *mcumgr_app_image_bin_filename(void)
{
    return mcumgr_app_bin_filename;
}

char *mcumgr_app_image_bin_version(void)
{
    return mcumgr_app_bin_version;
}

char *mcumgr_app_image_bin_boardname(void)
{
    return mcumgr_app_bin_boardname;
}

static void extract_fw_version(char *file_name)
{
    // 編集領域を初期化
    memset(mcumgr_app_bin_version, 0, sizeof(mcumgr_app_bin_version));
    memset(mcumgr_app_bin_boardname, 0, sizeof(mcumgr_app_bin_boardname));
    // ファイル名をバッファにコピー
    char buf[32];
    strcpy(buf, file_name);
    // ファイル名（app_update.PCA10095.0.4.0.bin）からバージョン情報を抽出して保持
    int ver = 0, rev = 0, sub = 0;
    char *p = strtok(buf, ".");
    if (p) {
        p = strtok(NULL, ".");
        if (p) {
            strncpy(mcumgr_app_bin_boardname, p, strlen(p));
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
    sprintf(mcumgr_app_bin_version, "%d.%d.%d", ver, rev, sub);
}

bool mcumgr_app_image_bin_filename_get(const char *bin_file_dir_path, const char *bin_file_name_prefix)
{
    DIR *dir = opendir(bin_file_dir_path);
    if (dir == NULL) {
        return false;
    }
    bool found = false;
    memset(mcumgr_app_bin_filename, 0, sizeof(mcumgr_app_bin_filename));
    struct dirent *dp = readdir(dir);
    while (dp != NULL) {
        // ファイル名に bin_file_name_prefix が含まれている場合
        if (strncmp(dp->d_name, bin_file_name_prefix, strlen(bin_file_name_prefix)) == 0) {
            // フルパスを編集して保持
            sprintf(mcumgr_app_bin_filename, "%s/%s", bin_file_dir_path, dp->d_name);
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

//
// nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
// .bin=512Kバイトと見積っています。
//
static uint8_t mcumgr_app_bin[524288];
static size_t  mcumgr_app_bin_size;

uint8_t *mcumgr_app_image_bin(void)
{
    return mcumgr_app_bin;
}

size_t mcumgr_app_image_bin_size(void)
{
    return mcumgr_app_bin_size;
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

bool mcumgr_app_image_bin_read(const char *bin_file_path)
{
    // データバッファ／サイズを初期化
    memset(mcumgr_app_bin, 0, sizeof(mcumgr_app_bin));
    mcumgr_app_bin_size = 0;
    // .binファイルを読込
    size_t max_size = sizeof(mcumgr_app_bin);
    if (read_app_image_file(bin_file_path, max_size, mcumgr_app_bin, &mcumgr_app_bin_size) == false) {
        return false;
    }
    return true;
}
