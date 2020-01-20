//
//  nrf52_app_image.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/06.
//
#ifndef nrf52_app_image_h
#define nrf52_app_image_h

#include <stdlib.h>
#include <stdbool.h>

// DFU対象ファイル名
#define NRF52_APP_DAT_FILE_NAME "nrf52840_xxaa.dat"
#define NRF52_APP_BIN_FILE_NAME "nrf52840_xxaa.bin"
#define NRF52_APP_ZIP_FILE_NAME "app_dfu_package"

uint8_t *nrf52_app_image_dat(void);
uint8_t *nrf52_app_image_bin(void);
size_t   nrf52_app_image_dat_size(void);
size_t   nrf52_app_image_bin_size(void);
char    *nrf52_app_image_zip_version(void);
char    *nrf52_app_image_zip_filename(void);
bool     nrf52_app_image_zip_filename_get(const char *zip_file_dir_path);
bool     nrf52_app_image_zip_read(const char *zip_file_path);

#endif /* nrf52_app_image_h */
