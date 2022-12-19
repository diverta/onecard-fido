/* 
 * File:   fido_maintenance_define.h
 * Author: makmorit
 *
 * Created on 2022/12/19, 9:40
 */
#ifndef FIDO_MAINTENANCE_DEFINE_H
#define FIDO_MAINTENANCE_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

// 管理コマンドの識別用
#define MNT_COMMAND_BASE                    0x40
#define MNT_COMMAND_GET_FLASH_STAT          0x42
#define MNT_COMMAND_GET_APP_VERSION         0x43
#define MNT_COMMAND_BOOTLOADER_MODE         0x45
#define MNT_COMMAND_ERASE_BONDING_DATA      0x46
#define MNT_COMMAND_SYSTEM_RESET            0x47
#define MNT_COMMAND_INSTALL_ATTESTATION     0xc8
#define MNT_COMMAND_RESET_ATTESTATION       0xc9
#define MNT_COMMAND_GET_TIMESTAMP           0x4a
#define MNT_COMMAND_SET_TIMESTAMP           0x4b
#define MNT_COMMAND_PAIRING_REQUEST         0x4c

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_DEFINE_H */
