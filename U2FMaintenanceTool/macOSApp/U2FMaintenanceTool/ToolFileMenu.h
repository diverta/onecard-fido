//
//  ToolFileMenu.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#ifndef ToolFileMenu_h
#define ToolFileMenu_h

#import "ToolCommon.h"

@protocol ToolFileMenuDelegate;

    @interface ToolFileMenu : NSObject

    @property (nonatomic, weak) id<ToolFileMenuDelegate> delegate;

    - (id)initWithDelegate:(id<ToolFileMenuDelegate>)delegate;
    - (void)toolCommandWillCreateFile:(Command)command;

    // クラス変数アクセス用
    - (void)setupCommand:(Command)command;
    - (Command)getCommand;
    - (void)setupOutputFilePath:(NSString *)outputFilePath;

    // ToolXxxxDelegate 共通インターフェース
    - (NSString *)processNameOfCommand;

@end

@protocol ToolFileMenuDelegate <NSObject>
    // ToolXxxxDelegate 共通インターフェース
    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)toolCommandDidFail:(NSString *)errorMessage;
    - (void)toolCommandDidSuccess;

@end

#endif /* ToolFileMenu_h */
