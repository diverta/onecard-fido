//
//  ToolCDCHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/01/01.
//
#import <Foundation/Foundation.h>
#import <IOKit/serial/IOSerialKeys.h>

#import "ToolCDCHelper.h"
#import "ToolLogFile.h"

@interface ToolCDCHelper ()

@end

@implementation ToolCDCHelper

    - (id)init {
        self = [super init];
        return self;
    }

#pragma mark - Public methods

    - (NSArray *)createACMDevicePathList {
        // シリアルデバイスのインスタンスを走査
        CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
        if (classesToMatch == NULL) {
            [[ToolLogFile defaultLogger] error:@"ToolCDCHelper: IOServiceMatching returned NULL"];
            return nil;
        }
        
        // シリアルデバイスの情報を取得
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
        io_iterator_t serviceIterator = 0;
        kern_return_t kernResult =
        IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &serviceIterator);
        if (kernResult != KERN_SUCCESS) {
            [[ToolLogFile defaultLogger] errorWithFormat:
             @"ToolCDCHelper: IOServiceGetMatchingServices returned %d", kernResult];
            return nil;
        }
        if (serviceIterator == 0) {
            [[ToolLogFile defaultLogger] error:@"ToolCDCHelper: serviceIterator not exist"];
            return nil;
        }
        
        // ACMシリアルデバイスのパス文字列を取得（複数件になるためリストに格納）
        NSArray *deviceList = [self generateACMDevicePathListFrom:serviceIterator];
        (void)IOObjectRelease(serviceIterator);
        return deviceList;
    }

#pragma mark - Private methods

    - (NSArray *)generateACMDevicePathListFrom:(io_iterator_t)serviceIterator {
        // 空のリストを作成
        NSMutableArray *deviceList = [NSMutableArray array];
        // 接続されているシリアルデバイスを走査
        io_object_t serialService;
        while ((serialService = IOIteratorNext(serviceIterator)) != 0) {
            NSString *serialTypeString = [self getSerialTypeString:serialService];
            if (serialTypeString) {
                if ([serialTypeString isEqualToString:@kIOSerialBSDModemType]) {
                    NSString *devicePathString = [self getDevicePathString:serialService];
                    if (devicePathString) {
                        // USB CDC ACMデバイスである場合、そのパスをリストに格納
                        [deviceList addObject:devicePathString];
                    }
                }
            }
            (void)IOObjectRelease(serialService);
        }
        // USB CDC ACMデバイスのパスが格納されたリストを戻す
        return deviceList;
    }

    - (NSString *)getSerialTypeString:(io_object_t)serialService {
        NSString *serialTypeString = nil;
        CFStringRef serialType = (CFStringRef)IORegistryEntryCreateCFProperty(serialService, CFSTR(kIOSerialBSDTypeKey), kCFAllocatorDefault, 0);
        if (serialType) {
            serialTypeString = [[NSString alloc] initWithString:(__bridge NSString *)serialType];
            CFRelease(serialType);
        }
        return serialTypeString;
    }

    - (NSString *)getDevicePathString:(io_object_t)serialService {
        NSString *devicePathString = nil;
        CFStringRef devicePath = (CFStringRef)IORegistryEntryCreateCFProperty(serialService, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
        if (devicePath) {
            devicePathString = [[NSString alloc] initWithString:(__bridge NSString *)devicePath];
            CFRelease(devicePath);
        }
        return devicePathString;
    }

@end
