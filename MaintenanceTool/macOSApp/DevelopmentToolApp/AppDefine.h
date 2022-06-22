//
//  AppDefine.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#ifndef AppDefine_h
#define AppDefine_h

// コマンド種別
typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_FIDO_ATTESTATION,
    COMMAND_FIDO_ATTESTATION_INSTALL,
    COMMAND_FIDO_ATTESTATION_INSTALL_REQUEST,
    COMMAND_FIDO_ATTESTATION_RESET,
    COMMAND_VIEW_APP_VERSION,
    COMMAND_VIEW_LOG_FILE,
} Command;

#endif /* AppDefine_h */
