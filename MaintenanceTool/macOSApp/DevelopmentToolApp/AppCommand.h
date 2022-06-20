//
//  AppCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/14.
//
#ifndef AppCommand_h
#define AppCommand_h

@protocol AppCommandDelegate;

@interface AppCommand : NSObject

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id delegate;

    - (id)initWithDelegate:(id)delegate;

@end

@protocol AppCommandDelegate <NSObject>

    - (void)enableButtonsOfMainUI:(bool)enable;
    - (void)notifyMessageToMainUI:(NSString *)message;

@end

#endif /* AppCommand_h */
