//
//  DFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/20.
//
#ifndef DFUCommand_h
#define DFUCommand_h

#import "AppCommand.h"

// 処理ステータス
typedef enum : NSInteger {
    DFU_ST_NONE = 0,
    DFU_ST_GET_CURRENT_VERSION,
    DFU_ST_TO_BOOTLOADER_MODE,
    DFU_ST_UPLOAD_PROCESS,
    DFU_ST_CANCELED,
    DFU_ST_RESET_DONE,
    DFU_ST_WAIT_FOR_BOOT,
    DFU_ST_CHECK_UPDATE_VERSION,
} DFUStatus;

@interface DFUCommandParameter : NSObject

    @property (nonatomic) TransportType     transportType;
    // 認証器からHID経由で取得したバージョン、基板名
    @property (nonatomic) NSString         *currentVersion;
    @property (nonatomic) NSString         *currentBoardname;
    // 更新イメージファイル名から取得したバージョン
    @property (nonatomic) NSString         *updateVersionFromImage;
    // 処理ステータス
    @property (nonatomic) NSInteger         dfuStatus;

@end

@interface DFUCommand : AppCommand

    - (void)DFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)isUSBHIDConnected;

@end

#endif /* DFUCommand_h */
