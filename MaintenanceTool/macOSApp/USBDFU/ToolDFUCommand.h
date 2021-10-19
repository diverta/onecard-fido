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

    - (id)initWithDelegate:(id)delegate;
    - (bool)checkUSBHIDConnection;
    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolHIDCommandRef:(id)toolHIDCommandRef;
    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)hidCommandDidDetectConnect:(id)toolHIDCommandRef;
    - (void)hidCommandDidDetectRemoval:(id)toolHIDCommandRef;
    - (void)notifyFirmwareVersion:(NSString *)strFWRev boardname:(NSString *)strHWRev;
    - (void)notifyBootloaderModeResponse:(NSData *)message CMD:(uint8_t)cmd;

    - (void)commandWillChangeToBootloaderMode;

@end

#endif /* ToolDFU_h */
