//
//  ToolDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#ifndef ToolDFU_h
#define ToolDFU_h

#import "usb_dfu_util.h"

@interface ToolDFUCommand : NSObject

    - (void)startDFUProcess;
    - (void)hidCommandDidDetectConnect:(id)toolHIDCommandRef;
    - (void)notifyFirmwareVersion:(NSString *)strFWRev;

@end

#endif /* ToolDFU_h */
