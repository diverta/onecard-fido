//
//  ToolFilePanel.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/14.
//
#ifndef ToolFilePanel_h
#define ToolFilePanel_h

#import <Foundation/Foundation.h>

@protocol ToolFilePanelDelegate;

@interface ToolFilePanel : NSObject

    @property (nonatomic, weak) id<ToolFilePanelDelegate> delegate;

    - (id)initWithDelegate:(id<ToolFilePanelDelegate>)delegate;
    - (void)panelWillSelectPath:(id)sender parentWindow:(NSWindow *)parentWindow
                     withPrompt:(NSString *)prompt withMessage:(NSString *)message withFileTypes:(NSArray<NSString *> *)fileTypes;
    - (void)panelWillSelectFolder:(id)sender parentWindow:(NSWindow *)parentWindow
                       withPrompt:(NSString *)prompt withMessage:(NSString *)message;

@end

@protocol ToolFilePanelDelegate <NSObject>

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath modalResponse:(NSInteger)modalResponse;

@end

#endif /* ToolFilePanel_h */
