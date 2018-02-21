//
//  ToolFileMenu.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#ifndef ToolFileMenu_h
#define ToolFileMenu_h

@interface ToolFileMenu : NSObject

@property (nonatomic) NSString *outputFilePath;

- (NSString *)getProcessMessage;

- (bool)createKeypairPemFile;
- (bool)createCertreqCsrFile;
- (bool)createSelfcrtCrtFile;

@end

#endif /* ToolFileMenu_h */
