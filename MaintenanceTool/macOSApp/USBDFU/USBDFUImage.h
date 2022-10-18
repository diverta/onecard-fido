//
//  USBDFUImage.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#ifndef USBDFUImage_h
#define USBDFUImage_h

@protocol USBDFUImageDelegate;

@interface USBDFUImage : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)readDFUImageFile:(DFUCommandParameter *)commandParameter;
    - (bool)dfuImageIsAvailable:(DFUCommandParameter *)commandParameter;

@end

@protocol USBDFUImageDelegate <NSObject>

    - (void)notifyCriticalErrorMessage:(NSString *)errorMessage informative:(NSString *)informativeMessage;
    - (void)notifyErrorMessage:(NSString *)errorMessage;

@end

#endif /* USBDFUImage_h */
