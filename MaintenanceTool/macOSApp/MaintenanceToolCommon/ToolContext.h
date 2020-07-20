//
//  ToolContext.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/07/16.
//
#ifndef ToolContext_h
#define ToolContext_h

#import "AppDelegate.h"

@interface ToolContext : NSObject
    // アプリケーション参照を保持
    + (ToolContext *)instance;
    - (void)setAppDelegateRef:(AppDelegate *)ref;

    // 認証器の設定値を保持
    @property (nonatomic) bool      bleScanAuthEnabled;

@end

#endif /* ToolContext_h */
