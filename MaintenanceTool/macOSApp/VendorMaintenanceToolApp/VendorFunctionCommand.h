//
//  VendorFunctionCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#ifndef VendorFunctionCommand_h
#define VendorFunctionCommand_h

#import "AppCommand.h"

@interface VendorFunctionCommandParameter : NSObject

    @property (nonatomic) Command       command;
    @property (nonatomic) NSString     *commandName;
    @property (nonatomic) NSString     *commandErrorMessage;
    @property (nonatomic) bool          commandSuccess;

    // 画面で選択された鍵・証明書ファイルパスを保持
    @property (nonatomic) NSString     *pkeyPemPath;
    @property (nonatomic) NSString     *certPemPath;

@end

@interface VendorFunctionCommand : AppCommand

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)commandWillPerformVendorFunction;

@end

#endif /* VendorFunctionCommand_h */
