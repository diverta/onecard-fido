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

    // スキャンしたQRコードの情報を保持
    @property (nonatomic) NSMutableDictionary          *parsedQRCodeInfo;
    // 解析時の検索開始インデックスを保持
    @property (nonatomic) NSUInteger                    offset;

@end

@implementation QRCodeUtil

    - (bool)hasScreenshotPermission {
        // 画面収録の許可があるかどうかを問い合わせる
        CGDisplayStreamRef streamRef;
        streamRef = CGDisplayStreamCreateWithDispatchQueue(
            CGMainDisplayID(), 1, 1, kCVPixelFormatType_32BGRA, nil, dispatch_get_main_queue(),
            ^(CGDisplayStreamFrameStatus status, uint64_t time, IOSurfaceRef frame, CGDisplayStreamUpdateRef ref) {
            }
        );
        // 画面収録の許可がない場合 false
        bool hasPermission = false;
        if (streamRef) {
            hasPermission = true;
            CFRelease(streamRef);
        } else {
            [[ToolLogFile defaultLogger] error:@"No permission for screen shot capture"];
        }
        return hasPermission;
    }

    - (bool)scanQRCodeFromScreenShot {
        // デスクトップのスクリーンショットを取得し、イメージを抽出
        CGImageRef screenShot = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionAll, kCGNullWindowID, kCGWindowImageDefault);
        CIImage *ciImage = [[CIImage alloc] initWithCGImage:screenShot];
        CFRelease(screenShot);
        
        // イメージからQRコードをキャプチャーし、メッセージを抽出
        NSString *messageString = [self extractQRMessageFrom:ciImage];
        if (messageString == nil) {
            [[ToolLogFile defaultLogger] debug:@"QR code not detected"];
            return false;
        }
        // 抽出されたメッセージを解析
        [self parseQRMessageFrom:messageString];
        [[ToolLogFile defaultLogger] debugWithFormat:@"QR code detected: %@", [self parsedQRCodeInfo]];
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

    - (void)parseQRMessageFrom:(NSString *)messageString {
        // 配列を初期化
        [self setParsedQRCodeInfo:[[NSMutableDictionary alloc] init]];
        // offsetを検索対象文字列の先頭に設定
        [self setOffset:0];
        // 検索用区切り文字列を設定
        NSArray<NSString *> *strings = @[@"://", @"/", @"?", @"&"];
        int i = 0;
        while (i < [strings count] && [self offset] < [messageString length]) {
            // 検索用区切り文字が出現するまでの範囲を取得
            NSString *string = [strings objectAtIndex:i];
            NSRange range = [self getRangeFrom:messageString fromOffset:[self offset] toString:string];
            if (range.location == NSNotFound) {
                break;
            }
            // 部分文字列を抽出し、連想配列に設定
            NSString *substring = [messageString substringWithRange:range];
            [self extractParameterFrom:substring parameterNo:i toDictionary:[self parsedQRCodeInfo]];
            // offsetを検索対象文字列の位置に更新
            [self setOffset:range.location + [substring length] + [string length]];
            // & の場合は見つからなくなるまで検索を続ける
            if ([string isEqualToString:@"&"] == false) {
                i++;
            }
        }
    }

    - (void)extractParameterFrom:(NSString *)parameterString parameterNo:(int)number toDictionary:(NSMutableDictionary *)parameters {
        switch (number) {
            case 0:
                // protocol
                [parameters setObject:parameterString forKey:@"protocol"];
                break;
            case 1:
                // OATH method
                [parameters setObject:parameterString forKey:@"method"];
                break;
            case 2:
                // OATH account
                [self extractAccountParameterFrom:parameterString parameterNo:number toDictionary:parameters];
                break;
            default:
                // OATH parameter
                [self extractOathParameterFrom:parameterString parameterNo:number toDictionary:parameters];
                break;
        }
    }

    - (void)extractAccountParameterFrom:(NSString *)parameterString parameterNo:(int)number toDictionary:(NSMutableDictionary *)parameters {
        NSArray<NSString *> *parameter = [self parameter:parameterString separatedBy:@":"];
        NSString *value;
        if ([parameter count] > 1) {
            value = [parameter objectAtIndex:1];
        } else {
            value = [parameter objectAtIndex:0];
        }
        [parameters setObject:value forKey:@"account"];
    }

    - (void)extractOathParameterFrom:(NSString *)parameterString parameterNo:(int)number toDictionary:(NSMutableDictionary *)parameters {
        // キーと値のペアでない場合は何もしない
        NSArray<NSString *> *parameter = [self parameter:parameterString separatedBy:@"="];
        if ([parameter count] != 2) {
            return;
        }
        // キーと値のペアを連想配列に設定
        [parameters setObject:[parameter objectAtIndex:1] forKey:[parameter objectAtIndex:0]];
    }

    - (NSArray<NSString *> *)parameter:(NSString *)parameterString separatedBy:(NSString *)separator {
        // 配列を初期化
        NSMutableArray<NSString *> *parameters = [[NSMutableArray alloc] init];
        // 指定されたインデックスから、指定の文字までの範囲をNSRangeで戻す
        NSRange found = [parameterString rangeOfString:separator];
        if (found.location == NSNotFound) {
            // 区切り文字列が無い場合は単一値を戻す
            [parameters addObject:parameterString];
        } else {
            // 区切り文字列の前後を配列で戻す
            NSString *key = [parameterString substringWithRange:NSMakeRange(0, found.location)];
            NSString *value = [parameterString substringFromIndex:found.location + [separator length]];
            [parameters addObject:key];
            [parameters addObject:value];
        }
        return parameters;
    }

    - (NSRange)getRangeFrom:(NSString *)messageString fromOffset:(NSUInteger)offset toString:(NSString *)terminator {
        // 指定されたインデックスから、指定の文字までの範囲をNSRangeで戻す
        NSRange search = NSMakeRange(offset, [messageString length] - offset);
        NSRange found = [messageString rangeOfString:terminator options:0 range:search];
        if (found.location == NSNotFound) {
            found.location = [messageString length];
        }
        NSRange range = NSMakeRange(offset, found.location - offset);
        return range;
    }

@end
