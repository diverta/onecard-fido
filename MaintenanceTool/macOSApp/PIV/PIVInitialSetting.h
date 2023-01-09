//
//  PIVInitialSetting.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/09.
//
#ifndef PIVInitialSetting_h
#define PIVInitialSetting_h

#import <Foundation/Foundation.h>

@interface PIVInitialSetting : NSObject

    - (void)generateChuidAndCcc;
    - (NSData *)getChuidAPDUData;
    - (NSData *)getCccAPDUData;

@end

#endif /* PIVInitialSetting_h */
