//
//  ToolUSBDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#ifndef ToolUSBDFUCommand_h
#define ToolUSBDFUCommand_h

#import "usb_dfu_util.h"
#import "ToolCommon.h"

@interface ToolUSBDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)checkUSBHIDConnection;
    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolHIDCommandRef:(id)toolHIDCommandRef;
    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)hidCommandDidDetectConnect:(Command)command forCommandRef:(id)ref;
    - (void)hidCommandDidDetectRemoval:(Command)command forCommandRef:(id)ref;
    - (void)hidCommandDidProcess:(Command)command CMD:(uint8_t)cmd response:(NSData *)response;
    - (void)commandWillChangeToBootloaderMode;

@end

#endif /* ToolUSBDFUCommand_h */
