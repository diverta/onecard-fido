//
//  ToolCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "ToolCommand.h"
#import "ToolCommonMessage.h"
#import "ToolPopupWindow.h"

// for web safe B64 encode & decode
#import "NSData+Base64.h"

@interface ToolCommand ()

    @property (nonatomic) NSData   *bleResponseData;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

    @property (nonatomic) NSDictionary *U2FRequestDict;
    @property (nonatomic) NSDictionary *U2FResponseDict;

@end

@implementation ToolCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            self.delegate = delegate;
        }
        return self;
    }

#pragma mark - Web Safe B64 encode & decode

- (NSString *)encodeWebSafeB64StringFrom:(NSData *)data {
    // Not web safe--->Web safeに変換された文字列を得る
    NSString *encodedData = [data base64EncodedStringWithSeparateLines:false];
    NSString *str = [encodedData
                     stringByReplacingOccurrencesOfString:@"+" withString:@"-"];
    NSString *webSafeB64String = [str
                                  stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    return webSafeB64String;
}

- (NSData *)decodeWebSafeB64StringFrom:(NSString *)webSafeB64String {
    // Web safe--->Not web safeに変換してからデコード
    NSString *str = [webSafeB64String
                     stringByReplacingOccurrencesOfString:@"-" withString:@"+"];
    NSString *b64String = [str
                           stringByReplacingOccurrencesOfString:@"_" withString:@"/"];
    NSData *decodedData = [NSData dataFromBase64String:b64String];
    return decodedData;
}

#pragma mark - Private methods

- (void)createCommandEraseBond {
    NSLog(@"Erase bonding information start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x40, 0x01, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setBleRequestArray:[NSArray arrayWithObject:commandData]];
}

- (void)createCommandEraseSkeyCert {
    NSLog(@"Erase secure key and certificate start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x40, 0x02, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setBleRequestArray:[NSArray arrayWithObject:commandData]];
}

- (NSData *)convertSkeyPem:(NSString *)skeyPemContents {
    // Base64エンコードされた秘密鍵文字列をデコード
    NSData *decodedPemData = [[NSData alloc]
                              initWithBase64EncodedString:skeyPemContents
                              options:NSDataBase64DecodingIgnoreUnknownCharacters];

    // デコードされたデータが39バイト未満の場合はエラー
    if ([decodedPemData length] < 39) {
        NSLog(@"Secure key has invalid length: %ld", [decodedPemData length]);
        [self toolCommandDidProcess:false message:@"鍵ファイルに格納された秘密鍵の長さが不正です。"];
        return nil;
    }

    // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
    const char *decodedPem = [decodedPemData bytes];
    if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
        NSLog(@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]);
        [self toolCommandDidProcess:false message:@"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"];
        return nil;
    }
    
    // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
    // 先頭からリトルエンディアン形式で配置しなおす。
    char keyBuffer[32];
    for (int i = 0; i < 32; i++) {
        keyBuffer[31 - i] = decodedPem[7 + i];
    }

    // NSDataに変換された秘密鍵を戻す
    NSData *skeyData = [[NSData alloc] initWithBytes:keyBuffer length:sizeof(keyBuffer)];
    return skeyData;
}

- (NSData *)readSkeyFromFile:(NSString *)skeyFilePath {
    // 鍵ファイルから読み込み
    NSError  *err;
    NSString *str = [NSString
                     stringWithContentsOfFile:skeyFilePath
                     encoding:NSUTF8StringEncoding
                     error:&err];
    if (err.code) {
        NSLog(@"Secure key file read error: %@", err.description);
        [self toolCommandDidProcess:false message:@"鍵ファイルを読み込むことができません。"];
        return nil;
    }
    
    // 読み込んだデータからヘッダー、フッターを除去
    NSArray *lines = [str componentsSeparatedByString:@"\n"];
    NSMutableString *pem = [NSMutableString string];
    bool headerFound = false;
    for (NSString *line in lines) {
        if ([line length] == 0) {
            continue;
        }
        if ([line compare:@"-----BEGIN EC PRIVATE KEY-----"] == NSOrderedSame) {
            headerFound = true;
            continue;
        }
        if ([line compare:@"-----END EC PRIVATE KEY-----"] == NSOrderedSame) {
            continue;
        }
        if (headerFound == true) {
            [pem appendString:line];
        }
    }

    // ヘッダーが見つからない場合はエラー
    if (headerFound == false) {
        NSLog(@"Secure key file has no header 'BEGIN EC PRIVATE KEY'");
        [self toolCommandDidProcess:false message:@"鍵ファイルの内容が不正です。"];
        return nil;
    }

    // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
    // 先頭からリトルエンディアン形式で配置しなおす。
    return [self convertSkeyPem:pem];
}

- (void)createCommandInstallSkey {
    NSLog(@"Install secure key start");
    
    // 鍵ファイルから秘密鍵（32バイト）を取得
    NSData *dataSkey = [self readSkeyFromFile:self.skeyFilePath];
    if (dataSkey == nil) {
        [self setBleRequestArray:nil];
        return;
    }

    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:dataSkey INS:0x40 P1:0x03];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (NSData *)readCertFromFile:(NSString *)certFilePath {
    // 証明書ファイルから読み込み
    NSData *data = [NSData dataWithContentsOfFile:certFilePath];
    if (data == nil || [data length] == 0) {
        [self toolCommandDidProcess:false message:@"証明書ファイルを読み込むことができません。"];
        return nil;
    }

    // 証明書ファイルの長さが68バイト未満の場合はエラー
    NSUInteger dataCertLength = [data length];
    if (dataCertLength < 68) {
        [self toolCommandDidProcess:false message:@"証明書ファイルに格納されたデータの長さが不正です。"];
        return nil;
    }

    NSLog(@"証明書ファイル(%ldバイト)を読込みました。%@", dataCertLength, data);
    return data;
}

- (NSArray<NSData *> *)generateCommandArrayFrom:(NSData *)dataForCommand {
    unsigned char initHeader[] = {0x83, 0x00, 0x00};
    unsigned char contHeader[] = {0x00};

    NSUInteger dataForCommandLength = [dataForCommand length];
    NSUInteger start    = 0;
    char       sequence = 0;

    NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
    while (start < dataForCommandLength) {
        NSMutableData *dataRequest = [NSMutableData alloc];
        NSData *dataHeader;
        
        NSUInteger strlen = dataForCommandLength - start;
        if (start == 0) {
            // 最大61バイト分取得する
            if (strlen > 61) {
                strlen = 61;
            }
            // BLEヘッダーにリクエストデータ長を設定する
            initHeader[1] = dataForCommandLength / 256;
            initHeader[2] = dataForCommandLength % 256;
            dataHeader = [[NSData alloc] initWithBytes:initHeader length:sizeof(initHeader)];

        } else {
            // 最大63バイト分取得する
            if (strlen > 63) {
                strlen = 63;
            }
            // BLEヘッダーにシーケンス番号を設定する
            contHeader[0] = sequence++;
            dataHeader = [[NSData alloc] initWithBytes:contHeader length:sizeof(contHeader)];
        }

        // スタート位置からstrlen文字分切り出して、ヘッダーに連結
        [dataRequest appendData:dataHeader];
        [dataRequest appendData:[dataForCommand subdataWithRange:NSMakeRange(start, strlen)]];
        [array addObject:dataRequest];
        
        // スタート位置を更新
        start += strlen;
    }
    
    return array;
}

- (NSData *)generateAPDUDataFrom:(NSData *)data INS:(unsigned char)ins P1:(unsigned char)p1 {
    // APDUを編集するための一時領域
    unsigned char apduHeader[] = {0x00, ins, p1, 0x00, 0x00, 0x00, 0x00};
    unsigned char apduFooter[] = {0x00, 0x00};
    
    // リクエストデータ長を設定
    NSUInteger dataCertLength = [data length];
    apduHeader[sizeof(apduHeader)-2] = dataCertLength / 256;
    apduHeader[sizeof(apduHeader)-1] = dataCertLength % 256;
    
    // ヘッダー＋データ＋フッターを連結し、APDUを作成
    NSMutableData *dataForRequest =
        [[NSMutableData alloc] initWithBytes:apduHeader length:sizeof(apduHeader)];
    [dataForRequest appendData:data];
    [dataForRequest appendBytes:apduFooter length:sizeof(apduFooter)];
    
    return dataForRequest;
}

- (void)createCommandInstallCert {
    NSLog(@"Install certificate start");

    // 証明書ファイルから内容を取得
    NSData *dataCert = [self readCertFromFile:self.certFilePath];
    if (dataCert == nil) {
        [self setBleRequestArray:nil];
        return;
    }

    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:dataCert INS:0x40 P1:0x04];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (NSData *)generateHexBytesFrom:(NSString *)hexString {
    unsigned int  hexInt;
    unsigned char byte;

    // 与えられたHEX文字列を２文字ずつ切り出し、バイトデータに変換する
    NSMutableData *convertedBytes = [[NSMutableData alloc] init];
    for (int i = 0; i < [hexString length]; i+=2) {
        NSString *tmp = [hexString substringWithRange:NSMakeRange(i, 2)];
        [[NSScanner scannerWithString:tmp] scanHexInt:&hexInt];
        byte = (unsigned char)hexInt;
        [convertedBytes appendBytes:&byte length:sizeof(byte)];
    }

    return convertedBytes;
}

- (NSMutableData *)createTestRequestData {
    // テストデータから、リクエストデータの先頭部分を生成
    NSData *challenge =
        [self generateHexBytesFrom:@"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
    NSData *appIDHash =
        [self generateHexBytesFrom:@"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];

    NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
    [requestData appendData:appIDHash];
    return requestData;
}

- (void)createCommandTestRegister {
    NSLog(@"Health check start");

    // テストデータを編集
    NSMutableData *requestData = [self createTestRequestData];
    
    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x01 P1:0x00];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (NSData *)getKeyHandleDataFrom:(NSData *)registerResponse {
    // Registerレスポンスからキーハンドル(67バイト目から65バイト)を切り出し
    return [registerResponse subdataWithRange:NSMakeRange(66, 65)];
}

- (void)createCommandTestAuthFrom:(NSData *)registerResponse P1:(unsigned char)p1 {
    // Registerレスポンスからキーハンドルを切り出し、テストデータに連結
    NSMutableData *requestData = [self createTestRequestData];
    [requestData appendData:[self getKeyHandleDataFrom:registerResponse]];
    
    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x02 P1:p1];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (bool)commandArrayIsBlank {
    if ([self bleRequestArray]) {
        if ([[self bleRequestArray] count]) {
            return false;
        }
    }
    return true;
}

- (void)createCommandU2FRegister:(Command)command
             appIdHashWebSafeB64:(NSString *)appIdHashWebSafeB64
             challengeWebSafeB64:(NSString *)challengeWebSafeB64
                         version:(NSString *)version {
    NSLog(@"U2F Register start");
    NSLog(@"appIdHashWebSafeB64[%@]", appIdHashWebSafeB64);
    NSLog(@"challengeWebSafeB64[%@]", challengeWebSafeB64);
    NSLog(@"ver[%@]",                 version);
    
    // U2Fリクエストパラメーターをデコード
    NSData *appIdHash = [self decodeWebSafeB64StringFrom:appIdHashWebSafeB64];
    NSData *challenge = [self decodeWebSafeB64StringFrom:challengeWebSafeB64];
    
    // U2Fリクエストデータを編集
    NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
    [requestData appendData:appIdHash];
    
    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x01 P1:0x00];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (void)createCommandU2FAuthentication:(Command)command
                   appIdHashWebSafeB64:(NSString *)appIdHashWebSafeB64
                   challengeWebSafeB64:(NSString *)challengeWebSafeB64
                   keyHandleWebSafeB64:(NSString *)keyHandleWebSafeB64
                               version:(NSString *)version {
    NSLog(@"U2F Authentication start");
    NSLog(@"appIdHashWebSafeB64[%@]", appIdHashWebSafeB64);
    NSLog(@"challengeWebSafeB64[%@]", challengeWebSafeB64);
    NSLog(@"keyhandleWebSafeB64[%@]", keyHandleWebSafeB64);
    NSLog(@"ver[%@]",                 version);
    
    // U2Fリクエストパラメーターをデコード
    NSData *appIdHash = [self decodeWebSafeB64StringFrom:appIdHashWebSafeB64];
    NSData *challenge = [self decodeWebSafeB64StringFrom:challengeWebSafeB64];
    NSData *keyHandle = [self decodeWebSafeB64StringFrom:keyHandleWebSafeB64];
    
    // U2Fリクエストデータを編集
    NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
    [requestData appendData:appIdHash];
    
    unsigned char keyHandleLenChar[] = {(unsigned char)[keyHandle length]};
    [requestData appendBytes:keyHandleLenChar length:sizeof(keyHandleLenChar)];
    [requestData appendData:keyHandle];

    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x02 P1:0x03];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (NSString *)U2FRequestTypeString {
    // 受信データのリクエスト種別を取得
    if ([self U2FRequestDict]) {
        return [[self U2FRequestDict] objectForKey:@"type"];
    } else {
        return @"";
    }
}

- (bool)isEnrollHelperRequest {
    return [[self U2FRequestTypeString] isEqualToString:@"enroll_helper_request"];
}

- (bool)isSignHelperRequest {
    return [[self U2FRequestTypeString] isEqualToString:@"sign_helper_request"];
}

- (NSDictionary *)getU2FRequestDictForKey:(NSString *)keyword {
    if ([self U2FRequestDict]) {
        NSArray *array = [[self U2FRequestDict] objectForKey:keyword];
        if (array && [array count]) {
            NSDictionary *dict = [array objectAtIndex:0];
            if (dict) {
                return dict;
            }
        }
    }
    return nil;
}

- (void)createCommandU2FProcess {
    // 受信データから各項目を取得し、リクエスト種別に応じた処理を実行
    if ([self isEnrollHelperRequest]) {
        NSDictionary *dict = [self getU2FRequestDictForKey:@"enrollChallenges"];
        if (dict) {
            [self createCommandU2FRegister:COMMAND_U2F_PROCESS
                       appIdHashWebSafeB64:[dict objectForKey:@"appIdHash"]
                       challengeWebSafeB64:[dict objectForKey:@"challengeHash"]
                                   version:[dict objectForKey:@"version"]];
        }
    } else if ([self isSignHelperRequest]) {
        NSDictionary *dict = [self getU2FRequestDictForKey:@"signData"];
        if (dict) {
            [self createCommandU2FAuthentication:COMMAND_U2F_PROCESS
                             appIdHashWebSafeB64:[dict objectForKey:@"appIdHash"]
                             challengeWebSafeB64:[dict objectForKey:@"challengeHash"]
                             keyHandleWebSafeB64:[dict objectForKey:@"keyHandle"]
                                         version:[dict objectForKey:@"version"]];
        }
    }
}

- (NSUInteger)getStatusWordFrom:(NSData *)bleResponseData {
    // BLEレスポンスデータから、ステータスワードを取得する
    NSUInteger length = [bleResponseData length];
    NSData *responseStatusWord = [bleResponseData subdataWithRange:NSMakeRange(length-2, 2)];
    unsigned char *statusWordChar = (unsigned char *)[responseStatusWord bytes];
    NSUInteger statusWord = statusWordChar[0] * 256 + statusWordChar[1];
    
    return statusWord;
}

- (void)createU2FResponseDictFrom:(NSData *)bleResponseData {
    // ステータスワードが正常の場合はリターンコードを０とする
    NSUInteger statusWord = [self getStatusWordFrom:bleResponseData];
    if (statusWord == 0x9000) {
        statusWord = 0;
    }
    NSNumber *statusWordNumber = [[NSNumber alloc] initWithUnsignedInteger:statusWord];
    
    // BLEからのレスポンスからステータスワードを除去し、Web Safe Base64エンコード
    NSUInteger length = [bleResponseData length];
    NSData *bleResponse = [bleResponseData subdataWithRange:NSMakeRange(0, length-2)];
    NSString *encodedResponse = [self encodeWebSafeB64StringFrom:bleResponse];
    
    // リクエスト種別に応じたレスポンスデータを生成
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    if ([self isEnrollHelperRequest]) {
        // レスポンスするversionはリクエストと同値を戻す
        NSDictionary *reqDict = [self getU2FRequestDictForKey:@"enrollChallenges"];
        NSString *version = [reqDict objectForKey:@"version"];
        // レスポンスデータを生成
        [dict setValue:@"enroll_helper_reply" forKey:@"type"];
        [dict setValue:version                forKey:@"version"];
        [dict setValue:statusWordNumber       forKey:@"code"];
        [dict setValue:encodedResponse        forKey:@"enrollData"];
        
    } else if ([self isSignHelperRequest]) {
        // レスポンスするappIdHash、challenge、keyHandle、versionはリクエストと同値を戻す
        NSDictionary *reqDict = [self getU2FRequestDictForKey:@"signData"];
        NSString *appIdHashWebSafeB64 = [reqDict objectForKey:@"appIdHash"];
        NSString *challengeWebSafeB64 = [reqDict objectForKey:@"challengeHash"];
        NSString *keyHandleWebSafeB64 = [reqDict objectForKey:@"keyHandle"];
        NSString *version             = [reqDict objectForKey:@"version"];
        // レスポンスデータを生成
        NSMutableDictionary *subdict = [[NSMutableDictionary alloc] init];
        [subdict setValue:version              forKey:@"version"];
        [subdict setValue:appIdHashWebSafeB64  forKey:@"appIdHash"];
        [subdict setValue:challengeWebSafeB64  forKey:@"challengeHash"];
        [subdict setValue:keyHandleWebSafeB64  forKey:@"keyHandle"];
        [subdict setValue:encodedResponse      forKey:@"signatureData"];
        [dict    setValue:@"sign_helper_reply" forKey:@"type"];
        [dict    setValue:statusWordNumber     forKey:@"code"];
        [dict    setValue:subdict              forKey:@"responseData"];
    }
    
    // レスポンスデータを保持
    [self setU2FResponseDict:dict];
}

#pragma mark - Private methods for setup

- (NSString *)createChromeSettingJsonString {
    // Native Messagingの相手となるエクステンションIDを列挙
    NSMutableArray *array = [[NSMutableArray alloc] initWithObjects:CHROME_EXTENSION_ID_URL, nil];
    
    // 実行可能ファイルのパスを取得
    NSArray *commandLineArgs = [[NSProcessInfo processInfo] arguments];
    NSString *pathString = [commandLineArgs objectAtIndex:0];
    
    // 項目設定
    NSMutableDictionary *jsonDictionary = [[NSMutableDictionary alloc] init];
    [jsonDictionary setValue:CHROME_NMHOST_NAME forKey:@"name"];
    [jsonDictionary setValue:CHROME_NMHOST_DESC forKey:@"description"];
    [jsonDictionary setValue:CHROME_NMHOST_TYPE forKey:@"type"];
    [jsonDictionary setValue:pathString         forKey:@"path"];
    [jsonDictionary setValue:array              forKey:@"allowed_origins"];
    
    // 文字列に変換
    NSError *error;
    NSData *jsonStringData = [NSJSONSerialization
                              dataWithJSONObject:jsonDictionary
                              options:NSJSONWritingPrettyPrinted
                              error:&error];
    if (jsonStringData) {
        NSString *jsonString = [[NSString alloc] initWithData:jsonStringData encoding:NSUTF8StringEncoding];
        return [jsonString stringByReplacingOccurrencesOfString:@"\\" withString:@""];
    } else {
        NSLog(@"Chrome Native Messaging Host JSON string create failed: %@", error);
        return nil;
    }
}

- (NSString *)prepareChromeSettingJsonDirectory {
    // JSONインストール先ディレクトリーを取得
    NSString *username = NSUserName();
    NSString *targetPath = CHROME_NMHOST_JSON_DIR;
    if ([username isEqualToString:@"root"] == false) {
        targetPath = [NSString stringWithFormat:@"%1$@%2$@", NSHomeDirectory(), CHROME_NMHOST_JSON_DIR];
    }
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:targetPath isDirectory:nil]) {
        NSLog(@"Chrome Native Messaging Host JSON Path exist: %@", targetPath);
    } else {
        // インストール先ディレクトリーがない場合は生成
        NSError *error;
        if ([[NSFileManager defaultManager] createDirectoryAtPath:targetPath
                withIntermediateDirectories:NO attributes:nil error:&error]) {
            NSLog(@"Chrome Native Messaging Host JSON Path created: %@", targetPath);
        } else {
            NSLog(@"Chrome Native Messaging Host JSON Path create failed: %@", error);
            return nil;
        }
    }

    return targetPath;
}

- (bool)writeChromeSettingJsonFileTo:(NSString *)targetPath jsonString:(NSString *)jsonString {
    // インストール先パスを編集し、JSONファイルを出力
    NSString *jsonFilePath = [NSString stringWithFormat:@"%1$@/%2$@.json", targetPath, CHROME_NMHOST_NAME];
    NSError *error;
    if ([jsonString writeToFile:jsonFilePath atomically:true
            encoding:NSUTF8StringEncoding error:&error]) {
        NSLog(@"Chrome Native Messaging Host JSON File created: %@", jsonFilePath);
    } else {
        NSLog(@"Chrome Native Messaging Host JSON File create failed: %@", error);
        return false;
    }

    return true;
}

- (void)setupChromeNativeMessaging {
    // 設定用のJSON文字列を生成
    NSString *jsonString = [self createChromeSettingJsonString];
    if (jsonString == nil) {
        [self toolCommandDidProcess:false message:@"設定用JSON文字列生成に失敗しました。処理を再試行してください。"];
        return;
    }

    // JSONインストール先ディレクトリーを取得
    NSString *targetPath = [self prepareChromeSettingJsonDirectory];
    if (targetPath == nil) {
        [self toolCommandDidProcess:false message:@"設定用JSONファイルインストール先ディレクトリー取得に失敗しました。処理を再試行してください。"];
        return;
    }
    
    // インストール先パスを編集し、JSONファイルを出力
    if ([self writeChromeSettingJsonFileTo:targetPath jsonString:jsonString] == false) {
        [self toolCommandDidProcess:false message:@"設定用JSONファイルの出力に失敗しました。処理を再試行してください。"];
        return;
    }
    
    // 処理正常終了をAppDelegateに通知
    [self toolCommandDidProcess:true message:@"Setup chrome native messaging end."];
}

#pragma mark - Public methods

- (void)setInstallParameter:(Command)command
          skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath {
    // インストール対象の鍵・証明書ファイルパスを保持
    [self setSkeyFilePath:skeyFilePath];
    [self setCertFilePath:certFilePath];
}

- (void)setU2FProcessParameter:(Command)command
      bleHelperMessages:(NSArray<NSDictionary *> *)bleHelperMessages {
    // Chromeエクステンションからの受信データを保持(最初の１件)
    if (bleHelperMessages && [bleHelperMessages count] == 1) {
        [self setU2FRequestDict:[bleHelperMessages objectAtIndex:0]];
        NSLog(@"setU2FProcessParameter: %@", [self U2FRequestDict]);
    }
}

- (NSString *)processNameOfCommand {
    // 現在実行中のコマンドに対応する名称を戻す
    NSString *processName;
    switch ([self command]) {
        case COMMAND_ERASE_BOND:
            processName = PROCESS_NAME_ERASE_BOND;
            break;
        case COMMAND_ERASE_SKEY_CERT:
            processName = PROCESS_NAME_ERASE_KEYCRT;
            break;
        case COMMAND_INSTALL_SKEY:
        case COMMAND_INSTALL_CERT:
            processName = PROCESS_NAME_INSTALL_KEYCRT;
            break;
        case COMMAND_TEST_REGISTER:
        case COMMAND_TEST_AUTH_CHECK:
        case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
        case COMMAND_TEST_AUTH_USER_PRESENCE:
            processName = PROCESS_NAME_HEALTHCHECK;
            break;
        case COMMAND_SETUP_CHROME_NATIVE_MESSAGING:
            processName = PROCESS_NAME_SETUP_CHROME;
            break;
        default:
            processName = nil;
            break;
    }
    return processName;
}

- (void)toolCommandWillCreateBleRequest:(Command)command {
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
        case COMMAND_ERASE_BOND:
            [self createCommandEraseBond];
            break;
        case COMMAND_ERASE_SKEY_CERT:
            [self createCommandEraseSkeyCert];
            break;
        case COMMAND_INSTALL_SKEY:
            [self createCommandInstallSkey];
            break;
        case COMMAND_TEST_REGISTER:
            [self createCommandTestRegister];
            break;
        case COMMAND_U2F_PROCESS:
            [self createCommandU2FProcess];
            break;
        default:
            [self setBleRequestArray:nil];
            break;
    }
    // コマンド生成時
    if ([self commandArrayIsBlank] == false) {
        [self.delegate toolCommandDidCreateBleRequest];
    }
}

- (bool)isResponseCompleted:(NSData *)responseData {
    // 受信データおよび長さを保持
    static NSUInteger     totalLength;
    static NSMutableData *receivedData;
    
    // 後続データの存在有無をチェック
    NSData *dataBLEHeader = [responseData subdataWithRange:NSMakeRange(0, 3)];
    unsigned char *bytesBLEHeader = (unsigned char *)[dataBLEHeader bytes];
    if (bytesBLEHeader[0] == 0x82) {
        // キープアライブの場合は引き続き次のレスポンスを待つ
        receivedData = nil;
        
    } else if (bytesBLEHeader[0] == 0x83) {
        // ヘッダーから全受信データ長を取得
        totalLength  = bytesBLEHeader[1] * 256 + bytesBLEHeader[2];
        // 4バイト目から後ろを切り出して連結
        NSData *tmp  = [responseData subdataWithRange:NSMakeRange(3, [responseData length] - 3)];
        receivedData = [[NSMutableData alloc] initWithData:tmp];
        
    } else {
        // 2バイト目から後ろを切り出して連結
        NSData *tmp  = [responseData subdataWithRange:NSMakeRange(1, [responseData length] - 1)];
        [receivedData appendData:tmp];
    }
    NSLog(@"Received response %@", responseData);
    
    if (receivedData && ([receivedData length] == totalLength)) {
        // 全受信データを保持
        [self setBleResponseData:[[NSData alloc] initWithData:receivedData]];
        [self.delegate notifyToolCommandMessage:@"レスポンスを受信しました。"];
        receivedData = nil;
        // 後続レスポンスがない
        return false;
        
    } else {
        // 後続レスポンスがある
        [self setBleResponseData:nil];
        return true;
    }
}

- (bool)checkStatusWordOfResponse {
    // レスポンスデータが揃っていない場合は終了
    if (![self bleResponseData]) {
        return false;
    }
    
    // ステータスワード(レスポンスの末尾２バイト)を取得
    NSUInteger statusWord = [self getStatusWordFrom:[self bleResponseData]];
    
    // 成功判定は、キーハンドルチェックの場合0x6985、それ以外は0x9000
    if ([self command] == COMMAND_TEST_AUTH_CHECK && statusWord == 0x6985) {
        return true;
    } else if (statusWord == 0x9000) {
        return true;
    }
    
    // nRF52側のFlash ROMがいっぱいになった場合のエラーである場合はその旨を通知
    if (statusWord == 0x9e01) {
        [self toolCommandDidProcess:false message:@"One CardのFlash ROM領域が一杯になり処理が中断されました(領域は自動再編成されます)。\n処理を再試行してください。"];
        return false;
    }
    
    // ステータスワードチェックがNGの場合
    [self toolCommandDidProcess:false message:@"BLEエラーが発生しました。処理を再試行してください。"];
    return false;
}

- (void)toolCommandWillProcessBleResponse {
    // Registerレスポンスは、３件のテストケースで共通使用するため、
    // ここで保持しておく必要がある
    static NSData *registerReponseData;
    
    // レスポンスをチェックし、内容がNGであれば処理終了
    if ([self checkStatusWordOfResponse] == false) {
        return;
    }
    
    // コマンドに応じ、以下の処理に分岐
    switch ([self command]) {
        case COMMAND_ERASE_BOND:
            [self toolCommandDidProcess:true message:@"Erase bonding information end"];
            break;
        case COMMAND_ERASE_SKEY_CERT:
            [self toolCommandDidProcess:true message:@"Erase secure key and certificate end"];
            break;
        case COMMAND_INSTALL_SKEY:
            NSLog(@"Install secure key end");
            [self setCommand:COMMAND_INSTALL_CERT];
            [self createCommandInstallCert];
            break;
        case COMMAND_INSTALL_CERT:
            [self toolCommandDidProcess:true message:@"Install certificate end"];
            break;
        case COMMAND_TEST_REGISTER:
            NSLog(@"Register test success");
            // Registerレスポンスを内部で保持して後続処理を実行
            registerReponseData = [[NSData alloc] initWithData:[self bleResponseData]];
            [self setCommand:COMMAND_TEST_AUTH_CHECK];
            [self createCommandTestAuthFrom:registerReponseData P1:0x07];
            break;
        case COMMAND_TEST_AUTH_CHECK:
            NSLog(@"Authenticate test (check) success");
            [self setCommand:COMMAND_TEST_AUTH_NO_USER_PRESENCE];
            [self createCommandTestAuthFrom:registerReponseData P1:0x08];
            break;
        case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            NSLog(@"Authenticate test (dont-enforce-user-presence-and-sign) success");
            [self setCommand:COMMAND_TEST_AUTH_USER_PRESENCE];
            [self createCommandTestAuthFrom:registerReponseData P1:0x03];
            break;
        case COMMAND_TEST_AUTH_USER_PRESENCE:
            registerReponseData = nil;
            NSLog(@"Authenticate test (enforce-user-presence-and-sign) success");
            [self toolCommandDidProcess:true message:@"Health check end"];
            break;
        case COMMAND_U2F_PROCESS:
            NSLog(@"U2F response received");
            [self setBleRequestArray:nil];
            [self createU2FResponseDictFrom:[self bleResponseData]];
            [self.delegate toolCommandDidReceive:[self U2FResponseDict]];
            break;
        default:
            break;
    }
    // コマンド生成時は後続処理を実行させる
    if ([self commandArrayIsBlank] == false) {
        [self.delegate toolCommandDidCreateBleRequest];
    }
}

- (void)toolCommandWillSetup:(Command)command {
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
        case COMMAND_SETUP_CHROME_NATIVE_MESSAGING:
            [self setupChromeNativeMessaging];
            break;
        default:
            break;
    }
}

- (void)toolCommandDidProcess:(bool)result message:(NSString *)message {
    // コマンド配列をブランクに初期化
    [self setBleRequestArray:nil];
    
    // 引数のメッセージを、処理成功時はコンソール出力、処理失敗時は画面出力
    if (result) {
        NSLog(@"%@", message);
    } else {
        [[self delegate] notifyToolCommandMessage:message];
    }

    // 処理終了メッセージを、テキストエリアとポップアップの両方に表示させる
    NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                     [self processNameOfCommand],
                     result? MSG_SUCCESS:MSG_FAILURE];
    [[self delegate] notifyToolCommandMessage:str];
    if (result) {
        [ToolPopupWindow informational:str informativeText:nil];
    } else {
        [ToolPopupWindow critical:str informativeText:nil];
    }
    
    // 処理終了をAppDelegateに通知
    [[self delegate] notifyToolCommandEnd];
}

@end
