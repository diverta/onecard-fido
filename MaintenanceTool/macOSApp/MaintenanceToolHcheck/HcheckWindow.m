//
//  HcheckWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppDefine.h"
#import "HcheckCommand.h"
#import "HcheckWindow.h"
#import "ToolCommonFunc.h"

@interface HcheckWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          hcheckCommandRef;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation HcheckWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setHcheckCommandRef:ref];
    }

    - (IBAction)buttonBLECtap2HealthCheckDidPress:(id)sender {
    }

    - (IBAction)buttonBLEU2FHealthCheckDidPress:(id)sender {
    }

    - (IBAction)buttonBLEPingTestDidPress:(id)sender {
    }

    - (IBAction)buttonHIDCtap2HealthCheckDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
    }

    - (IBAction)buttonHIDU2FHealthCheckDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
    }

    - (IBAction)buttonHIDPingTestDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
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
        HcheckCommand *command = (HcheckCommand *)[self hcheckCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

@end
