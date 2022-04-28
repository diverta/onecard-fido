//
//  QRCodeUtil.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/04/28.
//
#import <CoreImage/CoreImage.h>
#import <CoreGraphics/CoreGraphics.h>

#import "QRCodeUtil.h"
#import "ToolLogFile.h"

@interface QRCodeUtil ()
@end

@implementation QRCodeUtil

    - (bool)scanScreen {
        // TODO: 動作確認用の仮実装です。
        NSURL *url = [NSURL fileURLWithPath:@"/Users/makmorit/Downloads/qrcode.png"];
        CIImage *ciImage = [[CIImage alloc] initWithContentsOfURL:url];
        // イメージからQRコードをキャプチャーし、メッセージを抽出
        NSString *messageString = [self extractQRMessageFrom:ciImage];
        if (messageString == nil) {
            [[ToolLogFile defaultLogger] debug:@"QR code not detected"];
            return false;
        }
        // TODO: 抽出されたメッセージを解析
        [[ToolLogFile defaultLogger] debugWithFormat:@"QR code detected: %@", messageString];
        return true;
    }

    - (NSString *)extractQRMessageFrom:(CIImage *)ciImage {
        // イメージから解析情報を抽出
        CIContext *context = [[CIContext alloc] init];
        NSDictionary *options = @{CIDetectorAccuracy: CIDetectorAccuracyHigh};
        CIDetector *ciDetector = [CIDetector detectorOfType:CIDetectorTypeQRCode context:context options:options];
        // 解析できない場合は終了
        NSArray<CIFeature *> *features = [ciDetector featuresInImage:ciImage options:options];
        if ([features count] == 0) {
            return nil;
        }
        // QRコードを解析できる場合はデータを抽出
        for (CIFeature *feature in features) {
            if ([[feature type] isNotEqualTo:CIFeatureTypeQRCode]) {
                continue;
            }
            CIQRCodeFeature *qrCodeFeature = (CIQRCodeFeature *)feature;
            NSString *messageString = [qrCodeFeature messageString];
            return messageString;
        }
        // 解析できなかった場合はNULL
        return nil;
    }

@end
