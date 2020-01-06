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

uint8_t *nrf52_app_image_dat(void);
uint8_t *nrf52_app_image_bin(void);
size_t   nrf52_app_image_dat_size(void);
size_t   nrf52_app_image_bin_size(void);
bool     nrf52_app_image_dat_read(const char *dat_file_path);
bool     nrf52_app_image_bin_read(const char *bin_file_path);

#endif /* nrf52_app_image_h */
