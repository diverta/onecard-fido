//
//  ToolBLEHelper.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/01/09.
//
#import <Foundation/Foundation.h>
#import "ToolBLEHelper.h"
#import "ToolCommon.h"

@interface ToolBLEHelper ()

    @property (nonatomic) bool communicateAsChromeNative;
    @property (nonatomic) bool hasSentMessageToChrome;

@end

@implementation ToolBLEHelper

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate {
        self = [super init];
        if (self) {
            self.delegate = delegate;
            [self initProcessMode];
        }
        return self;
    }

    - (void)initProcessMode {
        // プログラム起動引数をチェック
        NSArray *commandLineArgs = [[NSProcessInfo processInfo] arguments];
        if ([commandLineArgs count] < 2) {
            [self setCommunicateAsChromeNative:false];
            return;
        }
        
        // ２番目の引数がChromeエクステンションIDになっている場合
        // Chromeエクステンションから実行されたと判定し、
        // ネイティブアプリとして動作するようにする
        NSString *extensionID = [commandLineArgs objectAtIndex:1];
        if ([extensionID compare:CHROME_EXTENSION_ID_URL] == NSOrderedSame) {
            [self setCommunicateAsChromeNative:true];
        } else {
            [self setCommunicateAsChromeNative:false];
        }
    }

    - (bool) bleHelperCommunicateAsChromeNative {
        return [self communicateAsChromeNative];
    }

#pragma mark - Functions for receive messages from chrome extension

    - (void)bleHelperWillSetStdinNotification {
        // Chromeからの標準入力を受信できるよう設定
        if ([self bleHelperCommunicateAsChromeNative]) {
            [self setStandardInputNotification];
        }
    }

    - (void)setStandardInputNotification {
        // 標準入力をメッセージとして受信できるよう設定
        NSFileHandle *_input = [NSFileHandle fileHandleWithStandardInput];
        [_input waitForDataInBackgroundAndNotify];
        
        // 標準入力を受信した時の処理
        NSNotificationCenter *_defaultCenter = [NSNotificationCenter defaultCenter];
        [_defaultCenter addObserverForName:NSFileHandleDataAvailableNotification
                                    object:_input queue:nil
                                usingBlock:^(NSNotification *notification){
                                    [self standardInputDidAvailable:_input];
                                }];
    }

    - (void)standardInputDidAvailable:(NSFileHandle *)input {
        // これ以上メッセージを受信できなくなった場合は終了
        NSData *data = [input availableData];
        if ([data length] == 0) {
            NSLog(@"End of message from chrome extension");
            return;
        }
        NSLog(@"Received Data:[%@]", data);
        
        // Chromeエクステンションにメッセージを返信した旨のフラグを初期化
        [self setHasSentMessageToChrome:false];

        // Chromeエクステンションから受信したメッセージを、AppDelegateへ転送
        [self.delegate bleHelperDidReceive:[self extractBLEHelperMessagesFrom:data]];
        
        // 次のメッセージが受信できるよう設定
        [input waitForDataInBackgroundAndNotify];
    }

    - (NSArray<NSDictionary *> *)extractBLEHelperMessagesFrom:(NSData *)data {
        // BLEメッセージ配列を初期化
        NSMutableArray<NSDictionary *> *array = [[NSMutableArray alloc] init];
        
        // エクステンションから1〜n件のメッセージが送信される可能性がある
        NSUInteger remaining_length = [data length];
        NSData    *remaining_data   = [data subdataWithRange:NSMakeRange(0, remaining_length)];
        while (remaining_length > 0) {
            // 受信メッセージ(JSON文字列)を連想配列に変換後、BLEメッセージ配列に追加
            NSData       *jsonData = [self extractSegmentDataFrom:remaining_data];
            NSDictionary *jsonDict = [self parseJsonStringFrom:jsonData];
            if (jsonDict) {
                [array addObject:jsonDict];
            }
            // 次の分割メッセージがない場合は終了
            if (remaining_length == 0) {
                break;
            }
            // 次の分割メッセージを取得
            NSUInteger _processed = 4 + [jsonData length];
            remaining_length = remaining_length - _processed;
            remaining_data = [remaining_data subdataWithRange:NSMakeRange(_processed, remaining_length)];
        }
        
        // 抽出されたBLEメッセージ配列の参照を戻す
        return array;
    }

    - (NSData *)extractSegmentDataFrom:(NSData *)stringDataFromChrome {
        // Chromeエクステンションから送信されたJSONテキストを抽出する。
        //   受信メッセージ(最初の4バイト分は、テキスト長を表す)
        //      250000007b2274657874223a223030316433326539....3535616530303030227d
        //   テキスト長
        //      25 00 00 00 ---> 0x25(=37バイトを表す)
        //   抽出されるテキスト(JSON形式)
        //      {"text":"001d32e9....55ae0000"}
        
        //   最初の４バイトから、受信メッセージテキスト長を取得
        NSData *_lengthData = [stringDataFromChrome subdataWithRange:NSMakeRange(0, 4)];
        unsigned char *length_char = (unsigned char *)[_lengthData bytes];
        NSUInteger _length = (length_char[1] * 256 + length_char[0]);
        
        //   最初の４バイトを除去し、受信メッセージテキストを抽出
        NSData *_dataString = [stringDataFromChrome subdataWithRange:NSMakeRange(4, _length)];
        return _dataString;
    }

    - (NSDictionary *)parseJsonStringFrom:(NSData *)jsonStringData {
        // JSON文字列データを連想配列に変換
        NSError *error;
        NSDictionary *jsonDictionary = [NSJSONSerialization
                                        JSONObjectWithData:jsonStringData
                                        options:NSJSONReadingAllowFragments
                                        error:&error];
        
        if (!jsonDictionary) {
            NSLog(@"%@", error);
        }
        
        return jsonDictionary;
    }

#pragma mark - Functions for send message to chrome extension

    - (void)bleHelperWillSend:(NSDictionary *)bleHelperMessage {
        // 連想配列-->JSON文字列-->Chromeエクステンションの指定形式に変換後、標準出力に書込み
        NSData *jsonStringData = [self createJsonStringFrom:bleHelperMessage];
        NSData *chromeMessageData = [self createCromeMessageFrom:jsonStringData];
        [[NSFileHandle fileHandleWithStandardOutput] writeData:chromeMessageData];
        // Chromeエクステンションにメッセージを返信した旨のフラグを設定
        [self setHasSentMessageToChrome:true];
        // アプリケーションに制御を戻す
        [[self delegate] bleHelperDidSend:chromeMessageData];
    }

    - (bool) bleHelperHasSentMessageToChrome {
        return [self hasSentMessageToChrome];
    }

    - (NSData *)createJsonStringFrom:(NSDictionary *)jsonDictionary {
        // JSON文字列データを連想配列に変換
        NSError *error;
        NSData *jsonStringData = [NSJSONSerialization
                                  dataWithJSONObject:jsonDictionary
                                  options:0
                                  error:&error];
        
        if (!jsonStringData) {
            NSLog(@"%@", error);
        }

        return jsonStringData;
    }

    - (NSData *)createCromeMessageFrom:(NSData *)jsonString {
        // Chromeエクステンションへ送信するメッセージを生成
        //   JSONテキスト(37バイト)
        //      {"text":"001d32e9....55ae0000"}
        //   JSONテキスト長からヘッダーバイトを作成
        //      0x25(=37バイトを表す) ---> 25 00 00 00
        //   送信メッセージ(最初の4バイト分は、テキスト長を表す)
        //      250000007b2274657874223a223030316433326539....3535616530303030227d
        NSUInteger _length = [jsonString length];

        unsigned char length_char[] = {0, 0, 0, 0};
        length_char[1] = _length / 256;
        length_char[0] = _length % 256;

        NSMutableData *_dataString = [[NSMutableData alloc]
                                      initWithBytes:length_char length:sizeof(length_char)];
        [_dataString appendData:jsonString];
        return _dataString;
    }

@end
