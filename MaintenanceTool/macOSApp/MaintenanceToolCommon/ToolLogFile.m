//
//  ToolLogFile.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/11/11.
//
#import <Foundation/Foundation.h>
#import "ToolLogFile.h"

static ToolLogFile *sharedInstance;

@interface ToolLogFile ()

    @property(nonatomic) NSString *logFilePath;

@end

@implementation ToolLogFile

#pragma mark - Methods for singleton

    + (ToolLogFile *)defaultLogger {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] init];
        });

        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Public methods

    - (void)error:(NSString *)logMessage {
        [self outputLogText:logMessage type:LOG_TYPE_ERROR];
    }

    - (void)errorWithFormat:(NSString *)format, ... {
        va_list args;
        va_start(args, format);
        [self logOutput:LOG_TYPE_ERROR withFormat:format arguments:args];
        va_end(args);
    }

    - (void)warn:(NSString *)logMessage {
        [self outputLogText:logMessage type:LOG_TYPE_WARN];
    }

    - (void)warnWithFormat:(NSString *)format, ... {
        va_list args;
        va_start(args, format);
        [self logOutput:LOG_TYPE_WARN withFormat:format arguments:args];
        va_end(args);
    }

    - (void)info:(NSString *)logMessage {
        [self outputLogText:logMessage type:LOG_TYPE_INFO];
    }

    - (void)infoWithFormat:(NSString *)format, ... {
        va_list args;
        va_start(args, format);
        [self logOutput:LOG_TYPE_INFO withFormat:format arguments:args];
        va_end(args);
    }

    - (void)debug:(NSString *)logMessage {
        [self outputLogText:logMessage type:LOG_TYPE_DEBUG];
    }

    - (void)debugWithFormat:(NSString *)format, ... {
        va_list args;
        va_start(args, format);
        [self logOutput:LOG_TYPE_DEBUG withFormat:format arguments:args];
        va_end(args);
    }

    - (void)dump:(NSString *)logMessage {
        [self outputLogText:logMessage type:LOG_TYPE_NONE];
    }

    - (void)hexdump:(NSData *)data {
        int length = (int)[data length];
        uint8_t *bytes = (uint8_t *)[data bytes];
        NSMutableString *hex = [[NSMutableString alloc] init];
        for (int i = 0; i < length; i++) {
            [hex appendFormat:@"%02x ", bytes[i]];
            if ((i % 16 == 15) && (i < length - 1)) {
                [hex appendString:@"\n"];
            }
        }
        [self outputLogText:hex type:LOG_TYPE_NONE];
    }

    - (NSString *)logFilePathString {
        // ログファイルパス文字列を戻す
        return [self logFilePath];
    }

#pragma mark - Private methods

    - (id)init {
        self = [super init];
        if (self) {
            // ログファイルパスを取得（ディレクトリーがない場合は新規に生成）
            [self setLogFilePath:[self getLogFilePath]];
        }
        return self;
    }

    - (void)logOutput:(LogType)type withFormat:(NSString *)format arguments:(va_list)argList {
        // 可変長引数からログテキストを編集
        NSString *logMessage = [[NSString alloc] initWithFormat:format arguments:argList];
        // 編集されたログテキストをファイル出力
        [self outputLogText:logMessage type:type];
    }

    - (void)outputLogText:(NSString *)logText type:(LogType)type {
        if (type == LOG_TYPE_NONE) {
            // ログファイルにメッセージを出力
            [self outputLogTextToFile:logText];
            
        } else {
            // メッセージの前方に、タイムスタンプとログ種別を付加して出力
            NSString *timestampStr = [self timestampString:[NSDate date]];
            NSString *logTypeStr = [self logTypeString:type];
            NSString *logMessage = [NSString stringWithFormat:@"%@ [%@] %@",
                                    timestampStr, logTypeStr, logText];
            [self outputLogTextToFile:logMessage];
        }
    }

    - (void)outputLogTextToFile:(NSString *)logText {
        // ログファイルのパスを取得
        NSString *path = [self logFilePath];
        // ログファイルにメッセージを出力
        @try {
            FILE* fp = fopen([path UTF8String], "a");
            if (fp != NULL) {
                fprintf(fp, "%s\n", [logText UTF8String]);
                fclose(fp);
            } else {
                NSLog(@"%@", logText);
            }
        } @catch (NSException *exception) {
            NSLog(@"outputLogText: %@", exception);
        }
    }

    - (NSString *)getLogFilePath {
        // ホームディレクトリー配下に生成
        NSString *dir = [NSString
                         stringWithFormat:@"%@/Library/Logs/Diverta/FIDO",
                         NSHomeDirectory()];
        // ディレクトリー存在チェック
        BOOL isDir = NO;
        NSFileManager *filemanager = [NSFileManager defaultManager];
        if ([filemanager fileExistsAtPath:dir isDirectory:&isDir] == false) {
            // ディレクトリーが存在しない場合は新規生成
            [filemanager createDirectoryAtPath:dir
                   withIntermediateDirectories:YES attributes:nil error:nil];
            NSLog(@"outputLogText: Directory created at %@", dir);
        }
        // ファイル名を連結して戻す
        NSString *path = [NSString
                          stringWithFormat:@"%@/MaintenanceTool.log", dir];
        return path;
    }

    - (NSString *)timestampString:(NSDate *)date {
        NSDateFormatter* df = [[NSDateFormatter alloc] init];
        NSCalendar* cal = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
        [df setCalendar:cal];
        [df setLocale:[NSLocale systemLocale]];
        [df setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSS"];
        NSString *timestampStr = [df stringFromDate:date];
        return timestampStr;
    }

    - (NSString *)logTypeString:(LogType)type {
        NSString *logTypeStr;
        switch (type) {
            case LOG_TYPE_ERROR:
                logTypeStr = @"error";
                break;
            case LOG_TYPE_WARN:
                logTypeStr = @"warn";
                break;
            case LOG_TYPE_INFO:
                logTypeStr = @"info";
                break;
            case LOG_TYPE_DEBUG:
                logTypeStr = @"debug";
                break;
            default:
                logTypeStr = @"";
                break;
        }
        return logTypeStr;
    }

@end
