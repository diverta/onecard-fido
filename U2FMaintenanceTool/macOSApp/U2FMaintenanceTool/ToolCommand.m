//
//  ToolCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "ToolCommand.h"

@interface ToolCommand ()

    @property (nonatomic) NSData   *bleResponseData;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

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
        [self.delegate toolCommandDidFail:@"鍵ファイルに格納された秘密鍵の長さが不正です。"];
        return nil;
    }

    // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
    const char *decodedPem = [decodedPemData bytes];
    if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
        NSLog(@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]);
        [self.delegate toolCommandDidFail:@"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"];
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
        [self.delegate toolCommandDidFail:@"鍵ファイルを読み込むことができません。"];
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
        [self.delegate toolCommandDidFail:@"鍵ファイルの内容が不正です。"];
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
        [self.delegate toolCommandDidFail:@"証明書ファイルを読み込むことができません。"];
        return nil;
    }

    // 証明書ファイルの長さが68バイト未満の場合はエラー
    NSUInteger dataCertLength = [data length];
    if (dataCertLength < 68) {
        [self.delegate toolCommandDidFail:@"証明書ファイルに格納されたデータの長さが不正です。"];
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

#pragma mark - Public methods

- (void)setKeyFilePath:(Command)command
          skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath {
    // インストール対象の鍵・証明書ファイルパスを保持
    [self setSkeyFilePath:skeyFilePath];
    [self setCertFilePath:certFilePath];
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

- (void)toolCommandWillProcessBleResponse {
    // レスポンスデータが揃っていない場合は終了
    if (![self bleResponseData]) {
        return;
    }
    
    // Registerレスポンスは、３件のテストケースで共通使用するため、
    // ここで保持しておく必要がある
    static NSData *registerReponseData;
    
    // レスポンスの末尾２バイトが0x9000でなければエラー扱い
    NSUInteger length = [[self bleResponseData] length];
    NSData *responseBytes = [[self bleResponseData] subdataWithRange:NSMakeRange(length-2, 2)];

    char successChars[] = {0x90, 0x00};
    switch ([self command]) {
        case COMMAND_TEST_AUTH_CHECK:
            // キーハンドルチェックの場合は成功判定バイトを差替
            successChars[0] = 0x69;
            successChars[1] = 0x85;
        default:
            break;
    }
    NSData *successBytes = [NSData dataWithBytes:successChars length:sizeof(successChars)];
    if ([responseBytes isEqualToData:successBytes] == false) {
        [self.delegate toolCommandDidFail:@"BLEエラーが発生しました。処理を再試行してください。"];
        return;
    }

    // コマンドに応じ、以下の処理に分岐
    switch ([self command]) {
        case COMMAND_ERASE_BOND:
            [self notifySuccess:@"Erase bonding information end"];
            break;
        case COMMAND_ERASE_SKEY_CERT:
            [self notifySuccess:@"Erase secure key and certificate end"];
            break;
        case COMMAND_INSTALL_SKEY:
            NSLog(@"Install secure key end");
            [self setCommand:COMMAND_INSTALL_CERT];
            [self createCommandInstallCert];
            break;
        case COMMAND_INSTALL_CERT:
            [self notifySuccess:@"Install certificate end"];
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
            [self notifySuccess:@"Health check end"];
            break;
        default:
            break;
    }
    // コマンド生成時は後続処理を実行させる
    if ([self commandArrayIsBlank] == false) {
        [self.delegate toolCommandDidCreateBleRequest];
    }
}

- (void)notifySuccess:(NSString *)successMessage {
    // コマンド配列をブランクにして、処理正常終了をAppDelegateに通知
    [self setBleRequestArray:nil];
    [self.delegate toolCommandDidSuccess];

    NSLog(@"%@", successMessage);
}

@end
