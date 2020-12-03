//
//  ToolPIVCommon.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVCommon_h
#define ToolPIVCommon_h

#define PIV_INS_SELECT_APPLICATION  0xa4
#define PIV_INS_VERIFY              0x20
#define PIV_INS_CHANGE_REFERENCE    0x24
#define PIV_INS_RESET_RETRY         0x2c
#define PIV_INS_PUT_DATA            0xdb
#define PIV_INS_AUTHENTICATE        0x87

#define YKPIV_INS_RESET             0xfb
#define YKPIV_INS_IMPORT_ASYMM_KEY  0xfe

#define PIV_KEY_PIN                 0x80
#define PIV_KEY_PUK                 0x81
#define PIV_KEY_AUTHENTICATION      0x9a
#define PIV_KEY_CARDMGM             0x9b
#define PIV_KEY_SIGNATURE           0x9c
#define PIV_KEY_KEYMGM              0x9d

#define PIV_OBJ_CAPABILITY          0x5fc107
#define PIV_OBJ_CHUID               0x5fc102
#define PIV_OBJ_AUTHENTICATION      0x5fc105
#define PIV_OBJ_SIGNATURE           0x5fc10a
#define PIV_OBJ_KEY_MANAGEMENT      0x5fc10b

#define PIV_ALG_3DES                0x03

#define TAG_DYNAMIC_AUTH_TEMPLATE   0x7c
#define TAG_AUTH_WITNESS            0x80
#define TAG_AUTH_CHALLENGE          0x81

#endif /* ToolPIVCommon_h */
