//
//  ToolCommandCrypto.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#ifndef ToolCommandCrypto_h
#define ToolCommandCrypto_h

@interface ToolCommandCrypto : NSObject

@property (nonatomic) NSString *outputFilePath;

- (NSString *)getProcessMessage;

- (bool)createKeypairPemFile;
- (bool)createCertreqCsrFile;
- (bool)createSelfcrtCrtFile;

@end

#endif /* ToolCommandCrypto_h */
