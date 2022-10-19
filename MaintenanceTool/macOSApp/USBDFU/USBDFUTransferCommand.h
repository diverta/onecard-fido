//
//  USBDFUTransferCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#ifndef USBDFUTransferCommand_h
#define USBDFUTransferCommand_h

@protocol USBDFUTransferCommandDelegate;

@interface USBDFUTransferCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)invokeTransferWithParamRef:(id)ref;

@end

@protocol USBDFUTransferCommandDelegate <NSObject>

    - (void)transferCommandDidTerminate:(bool)success;

@end

#endif /* USBDFUTransferCommand_h */
