//
//  ToolFileMenu.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#ifndef ToolFileMenu_h
#define ToolFileMenu_h

#import "ToolCommon.h"

@interface ToolFileMenu : NSObject

@property (nonatomic) NSString *outputFilePath;
@property (nonatomic) Command   command;

- (NSString *)getProcessMessage;

- (bool)createKeypairPemFile;
- (bool)createCertreqCsrFile;
- (bool)createSelfcrtCrtFile;

@end

#endif /* ToolFileMenu_h */
