//
//  AppCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/07.
//
#ifndef AppCommand_h
#define AppCommand_h

#import "AppDefine.h"

@protocol AppCommandDelegate;

@interface AppCommand : NSObject

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id          delegate;
    // 実行コマンドを保持
    @property (nonatomic) Command           command;
    @property (nonatomic) NSString         *commandName;

    - (id)initWithDelegate:(id)delegate;
    - (void)notifyCommandStarted:(NSString *)processNameOfCommand;
    - (void)notifyCommandTerminated:(NSString *)processNameOfCommand message:(NSString *)message success:(bool)success fromWindow:(NSWindow *)window;

@end

@protocol AppCommandDelegate <NSObject>

    - (void)enableButtonsOfMainUI:(bool)enable;
    - (void)notifyMessageToMainUI:(NSString *)message;

@end

#endif /* AppCommand_h */
