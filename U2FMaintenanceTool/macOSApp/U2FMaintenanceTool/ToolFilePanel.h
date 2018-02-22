//
//  ToolFilePanel.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#ifndef ToolFilePanel_h
#define ToolFilePanel_h

@protocol ToolFilePanelDelegate;

@interface ToolFilePanel : NSObject

    @property (nonatomic, weak) id<ToolFilePanelDelegate> delegate;

    - (id)initWithDelegate:(id<ToolFilePanelDelegate>)delegate;

#pragma mark for NSOpenPanel
    - (void)prepareOpenPanel:(NSString *)prompt message:(NSString *)message
                   fileTypes:(NSArray<NSString *> *)fileTypes;
    - (void)panelWillSelectPath:(id)sender parentWindow:(NSWindow *)parentWindow;

#pragma mark for NSSavePanel
    - (void)prepareSavePanel:(NSString *)prompt message:(NSString *)message
                    fileName:(NSString *)fileName
                   fileTypes:(NSArray<NSString *> *)fileTypes;
    - (void)panelWillCreatePath:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

@protocol ToolFilePanelDelegate <NSObject>

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath;
    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath;

@end

#endif /* ToolFilePanel_h */
