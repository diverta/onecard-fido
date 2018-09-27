//
//  ToolCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "ToolCommand.h"
#import "ToolCommonMessage.h"

// for web safe B64 encode & decode
#import "NSData+Base64.h"

@interface ToolCommand ()

    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;
    @property (nonatomic) NSData   *hidHelperMessage;

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

- (void)createCommandPairing {
    NSLog(@"Pairing start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x45, 0x00, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setBleRequestArray:[NSArray arrayWithObject:commandData]];
}

- (void)createCommandEraseSkeyCert {
    NSLog(@"Erase secure key and certificate start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x42, 0x00, 0x00};
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
        [self toolCommandDidProcess:false message:MSG_INVALID_SKEY_LENGTH_IN_PEM];
        return nil;
    }

    // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
    const char *decodedPem = [decodedPemData bytes];
    if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
        NSLog(@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]);
        [self toolCommandDidProcess:false message:MSG_INVALID_SKEY_HEADER_IN_PEM];
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
        [self toolCommandDidProcess:false message:MSG_CANNOT_READ_SKEY_PEM_FILE];
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
        [self toolCommandDidProcess:false message:MSG_INVALID_SKEY_CONTENT_IN_PEM];
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
    NSData *dataForRequest = [self generateAPDUDataFrom:dataSkey INS:0x43 P1:0x00];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}

- (NSData *)readCertFromFile:(NSString *)certFilePath {
    // 証明書ファイルから読み込み
    NSData *data = [NSData dataWithContentsOfFile:certFilePath];
    if (data == nil || [data length] == 0) {
        [self toolCommandDidProcess:false message:MSG_CANNOT_READ_CERT_CRT_FILE];
        return nil;
    }

    // 証明書ファイルの長さが68バイト未満の場合はエラー
    NSUInteger dataCertLength = [data length];
    if (dataCertLength < 68) {
        [self toolCommandDidProcess:false message:MSG_INVALID_CERT_LENGTH_IN_CRT];
        return nil;
    }

    NSLog(MSG_READ_NBYTES_FROM_CRT_FILE, dataCertLength, data);
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
    NSData *dataForRequest = [self generateAPDUDataFrom:dataCert INS:0x44 P1:0x00];
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

- (void)createCommandU2FHIDProcess {
    // HIDデバイスから転送されたAPDUより、分割送信のために64バイトごとのコマンド配列を作成
    [self setBleRequestArray:[self generateCommandArrayFrom:[self hidHelperMessage]]];
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

- (void)setU2FHIDProcessParameter:(Command)command
                 hidHelperMessage:(NSData *)hidHelperMessage {
    // HIDデバイスからの受信データを保持
    [self setHidHelperMessage:hidHelperMessage];
    NSLog(@"setU2FHIDProcessParameter: %@", [self hidHelperMessage]);
}

- (void)toolCommandWillCreateBleRequest:(Command)command {
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
        case COMMAND_ERASE_SKEY_CERT:
            [self createCommandEraseSkeyCert];
            break;
        case COMMAND_INSTALL_SKEY:
            [self createCommandInstallSkey];
            break;
        case COMMAND_TEST_REGISTER:
            [self createCommandTestRegister];
            break;
        case COMMAND_U2F_HID_PROCESS:
            [self createCommandU2FHIDProcess];
            break;
        case COMMAND_PAIRING:
            [self createCommandPairing];
            break;
        default:
            [self setBleRequestArray:nil];
            break;
    }
    // コマンド生成時
    if ([self commandArrayIsBlank] == false) {
        // コマンド開始メッセージを画面表示し、デリゲートに制御を戻す
        NSString *message = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                             [ToolCommon processNameOfCommand:command]];
        [[self delegate] notifyToolCommandMessage:message];
        [[self delegate] toolCommandDidCreateBleRequest];
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
        
    } else if (bytesBLEHeader[0] == 0x81 || bytesBLEHeader[0] == 0x83) {
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
    if (statusWord == 0x6985) {
        return true;
    } else if (statusWord == 0x9000) {
        return true;
    }
    
    // invalid keyhandleエラーである場合はその旨を通知
    if (statusWord == 0x6a80) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_KEYHANDLE_ERROR];
        return false;
    }
    
    // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
    if (statusWord == 0x9402) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_SKEYNOEXIST_ERROR];
        return false;
    }

    // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
    if (statusWord == 0x9601) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_PAIRINGMODE_ERROR];
        return false;
    }
    
    // ステータスワードチェックがNGの場合
    [self toolCommandDidProcess:false message:MSG_OCCUR_UNKNOWN_BLE_ERROR];
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
        case COMMAND_PAIRING:
            [self toolCommandDidProcess:true message:@"Pairing end"];
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
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_REGISTER_SUCCESS];
            NSLog(@"Register test success");
            // Registerレスポンスを内部で保持して後続処理を実行
            registerReponseData = [[NSData alloc] initWithData:[self bleResponseData]];
            [self setCommand:COMMAND_TEST_AUTH_CHECK];
            [self createCommandTestAuthFrom:registerReponseData P1:0x07];
            // 後続のU2F Authenticateを開始する前に、
            // One CardのMAIN SWを押してもらうように促すメッセージを表示
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_START];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT1];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT2];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT3];
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
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_SUCCESS];
            registerReponseData = nil;
            NSLog(@"Authenticate test (enforce-user-presence-and-sign) success");
            [self toolCommandDidProcess:true message:@"Health check end"];
            break;
        case COMMAND_U2F_HID_PROCESS:
            [self toolCommandDidProcess:true message:@"U2F response received"];
            break;
        default:
            break;
    }
    // コマンド生成時は後続処理を実行させる
    if ([self commandArrayIsBlank] == false) {
        [self.delegate toolCommandDidCreateBleRequest];
    }
}

- (void)toolCommandDidProcess:(bool)result message:(NSString *)message {
    // コマンド配列をブランクに初期化
    [self setBleRequestArray:nil];
    
    // 引数のメッセージをコンソール出力
    if (message) {
        NSLog(@"%@", message);
    }
    
    // 画面処理時は、処理終了とメッセージ文言をAppDelegateに戻す
    [[self delegate] toolCommandDidProcess:[self command] result:result
                                   message:message];
}

- (NSDictionary *)getU2FResponseDict {
    if ([self U2FResponseDict]) {
        return [self U2FResponseDict];
    } else {
        return [[NSDictionary alloc] init];
    }
}

@end
