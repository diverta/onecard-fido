//
//  ToolCDCHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/01.
//
#ifndef ToolCDCHelper_h
#define ToolCDCHelper_h

@interface ToolCDCHelper : NSObject

    - (NSArray *)createACMDevicePathList;
    - (bool)connectDeviceTo:(NSString *)ACMDevicePath;
    - (bool)writeToDevice:(NSData *)data;
    - (NSData *)readFromDevice;
    - (void)disconnectDevice;

@end

#endif /* ToolCDCHelper_h */
