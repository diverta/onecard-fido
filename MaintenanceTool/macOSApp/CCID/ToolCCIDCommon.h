//
//  ToolCCIDCommon.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#ifndef ToolCCIDCommon_h
#define ToolCCIDCommon_h

#define PIV_INS_SELECT_APPLICATION  0xa4
#define PIV_INS_VERIFY              0x20
#define PIV_INS_CHANGE_REFERENCE    0x24
#define PIV_INS_RESET_RETRY         0x2c
#define PIV_INS_PUT_DATA            0xdb

#define YKPIV_INS_RESET             0xfb
#define YKPIV_INS_IMPORT_ASYMM_KEY  0xfe

#define SW_SEC_STATUS_NOT_SATISFIED 0x6982
#define SW_ERR_AUTH_BLOCKED         0x6983
#define SW_SUCCESS                  0x9000

#define PIV_KEY_PIN                 0x80
#define PIV_KEY_PUK                 0x81

#endif /* ToolCCIDCommon_h */
