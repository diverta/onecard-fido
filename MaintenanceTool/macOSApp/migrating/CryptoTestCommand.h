//
//  CryptoTestCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#ifndef CryptoTestCommand_h
#define CryptoTestCommand_h

#import <Foundation/Foundation.h>

@interface CryptoTestCommand : NSObject

    - (void)testECKey;
    - (void)testAES256CBC;
    - (void)testTripleDES;

@end

#endif /* CryptoTestCommand_h */
