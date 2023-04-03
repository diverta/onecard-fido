//
//  SecurityTestCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/04/03.
//
#ifndef SecurityTestCommand_h
#define SecurityTestCommand_h

#import <Foundation/Foundation.h>

@interface SecurityTestCommand : NSObject

    - (void)testECDHWithSample;
    - (void)testECKey;

@end

#endif /* SecurityTestCommand_h */
