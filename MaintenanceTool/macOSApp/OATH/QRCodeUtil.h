//
//  QRCodeUtil.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/04/28.
//
#ifndef QRCodeUtil_h
#define QRCodeUtil_h

@interface QRCodeUtil : NSObject

    + (bool)hasScreenshotPermission;
    + (NSString *)scanQRCodeFromScreenShot;

    - (QRCodeUtil *)initWithQRMessageString:(NSString *)messageString;

@end

#endif /* QRCodeUtil_h */
