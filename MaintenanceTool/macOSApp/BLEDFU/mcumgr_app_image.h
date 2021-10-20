//
//  mcumgr_app_image.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/20.
//
#ifndef mcumgr_app_image_h
#define mcumgr_app_image_h

#include <stdlib.h>
#include <stdbool.h>

char       *mcumgr_app_image_bin_filename(void);
char       *mcumgr_app_image_bin_version(void);
char       *mcumgr_app_image_bin_boardname(void);
bool        mcumgr_app_image_bin_filename_get(const char *bin_file_dir_path, const char *bin_file_name_prefix);

#endif /* mcumgr_app_image_h */
