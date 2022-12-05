//
//  UtilityWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#import "AppDefine.h"
#import "UtilityCommand.h"
#import "UtilityWindow.h"
#import "ToolCommonFunc.h"

@interface UtilityWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonFlashROMInfo;
    @property (assign) IBOutlet NSButton                   *buttonFWVersionInfo;
    @property (assign) IBOutlet NSButton                   *buttonToolVersionInfo;
    @property (assign) IBOutlet NSButton                   *buttonViewLogFile;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          utilityCommandRef;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation UtilityWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setUtilityCommandRef:ref];
    }

    - (IBAction)buttonRTCCSettingDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_RTCC_SETTING];
    }

    - (IBAction)buttonFlashROMInfoDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_HID_GET_FLASH_STAT];
    }

    - (IBAction)buttonFWVersionInfoDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_HID_GET_VERSION_INFO];
    }

    - (IBAction)buttonToolVersionInfoDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_VIEW_APP_VERSION];
    }

    - (IBAction)buttonViewLogFileDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_VIEW_LOG_FILE];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel withCommand:COMMAND_NONE];
    }

    - (void)terminateWindow:(NSModalResponse)response withCommand:(Command)command {
        // 実行コマンドを保持
        [self setCommand:command];
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        UtilityCommand *command = (UtilityCommand *)[self utilityCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

@end
