//
//  USBDFUImage.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#ifndef USBDFUImage_h
#define USBDFUImage_h

@interface USBDFUImage : NSObject

    // 処理失敗時のエラーメッセージを保持
    @property (nonatomic) NSString                 *errorMessage;

    // 関数群
    - (bool)readDFUImageFile:(DFUCommandParameter *)commandParameter;

@end

#endif /* USBDFUImage_h */
