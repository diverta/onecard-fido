//
//  ToolFileMenu.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#ifndef ToolFileMenu_h
#define ToolFileMenu_h

@protocol ToolFileMenuDelegate;

    @interface ToolFileMenu : NSObject

    @property (nonatomic, weak) id<ToolFileMenuDelegate> delegate;

    - (id)initWithDelegate:(id<ToolFileMenuDelegate>)delegate;
    - (void)toolFileMenuWillCreateFile:(id)sender;

@end

@protocol ToolFileMenuDelegate <NSObject>

    - (void)notifyToolFileMenuMessage:(NSString *)message;
    - (void)notifyToolFileMenuEnd;

@end

#endif /* ToolFileMenu_h */
