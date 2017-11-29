//
//  ToolCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "ToolCommand.h"

@implementation ToolCommand

#pragma mark - Private methods

- (void)createCommandEraseBond {
    NSLog(@"Erase bonding information start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x40, 0x01, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setCommandArray:[NSArray arrayWithObject:commandData]];
}

- (void)createCommandEraseSkeyCert {
    NSLog(@"Erase secure key and certificate start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x40, 0x02, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setCommandArray:[NSArray arrayWithObject:commandData]];
}

- (NSData *)convertSkeyPem:(NSString *)skeyPemContents {
    // Base64エンコードされた秘密鍵文字列をデコード
    NSData *decodedPemData = [[NSData alloc]
                              initWithBase64EncodedString:skeyPemContents
                              options:NSDataBase64DecodingIgnoreUnknownCharacters];

    // デコードされたデータが39バイト未満の場合はエラー
    if ([decodedPemData length] < 39) {
        NSLog(@"Secure key has invalid length: %ld", [decodedPemData length]);
        [self setLastOccuredErrorMessage:@"鍵ファイルに格納された秘密鍵の長さが不正です。"];
        return nil;
    }

    // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
    const char *decodedPem = [decodedPemData bytes];
    if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
        NSLog(@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]);
        [self setLastOccuredErrorMessage:@"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"];
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
        [self setLastOccuredErrorMessage:@"鍵ファイルを読み込むことができません。"];
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
        [self setLastOccuredErrorMessage:@"鍵ファイルの内容が不正です。"];
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
        [self setCommandArray:nil];
        return;
    }

    // 書き込むコマンドを編集
    unsigned char arrHeader[] = {0x83, 0x00, 0x29, 0x00, 0x40, 0x03, 0x00, 0x00, 0x00, 0x20};
    unsigned char arrFooter[] = {0x00, 0x00};
    NSData *dataHeader = [[NSData alloc] initWithBytes:arrHeader length:sizeof(arrHeader)];
    NSData *dataFooter = [[NSData alloc] initWithBytes:arrFooter length:sizeof(arrFooter)];
    
    NSMutableData *dataRequest = [NSMutableData alloc];
    [dataRequest appendData:dataHeader];
    [dataRequest appendData:dataSkey];
    [dataRequest appendData:dataFooter];

    [self setCommandArray:[NSArray arrayWithObject:dataRequest]];
}

- (NSData *)readCertFromFile:(NSString *)certFilePath {
    // 証明書ファイルから読み込み
    NSData *data = [NSData dataWithContentsOfFile:certFilePath];
    if (data == nil || [data length] == 0) {
        [self setLastOccuredErrorMessage:@"証明書ファイルを読み込むことができません。"];
        return nil;
    }

    // 証明書ファイルの長さが68バイト未満の場合はエラー
    NSUInteger dataCertLength = [data length];
    if (dataCertLength < 68) {
        [self setLastOccuredErrorMessage:@"証明書ファイルに格納されたデータの長さが不正です。"];
        return nil;
    }

    NSLog(@"証明書ファイル(%ldバイト)を読込みました。%@", dataCertLength, data);
    return data;
}

- (NSArray<NSData *> *)generateCommandArray:(NSData *)dataForCommand {
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

- (NSData *)generateAPDUData:(NSData *)data INS:(unsigned char)ins P1:(unsigned char)p1 {
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
        [self setCommandArray:nil];
        return;
    }

    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUData:dataCert INS:0x40 P1:0x04];
    [self setCommandArray:[self generateCommandArray:dataForRequest]];
}

- (NSData *)generateBytesFromHexString:(NSString *)hexString {
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

- (void)createCommandCheckHealth {
    NSLog(@"Health check start");

    // テストデータを編集
    NSData *challenge =
        [self generateBytesFromHexString:@"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
    NSData *appIDHash =
        [self generateBytesFromHexString:@"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];
    NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
    [requestData appendData:appIDHash];

    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUData:requestData INS:0x01 P1:0x00];
    [self setCommandArray:[self generateCommandArray:dataForRequest]];
}

- (bool)commandArrayIsBlank {
    if (self.commandArray) {
        return false;
    }
    if ([self.commandArray count]) {
        return false;
    }
    return true;
}

#pragma mark - Public methods

- (bool)createCommandArray {
    [self setLastOccuredErrorMessage:nil];
    [self setCommandSuccess:false];

    // コマンドに応じ、以下の処理に分岐
    switch (self.command) {
        case COMMAND_ERASE_BOND:
            [self createCommandEraseBond];
            break;
        case COMMAND_ERASE_SKEY_CERT:
            [self createCommandEraseSkeyCert];
            break;
        case COMMAND_INSTALL_SKEY:
            [self createCommandInstallSkey];
            break;
        case COMMAND_INSTALL_CERT:
            [self createCommandInstallCert];
            break;
        case COMMAND_CHECK_HEALTH:
            [self createCommandCheckHealth];
            break;
        default:
            [self setCommandArray:nil];
            break;
    }

    // コマンド生成失敗時は処理中止
    if ([self commandArrayIsBlank]) {
        return false;
    }
    
    return true;
}

- (bool)doWithResponseValue:(NSData *)responseValue {
    // レスポンスの末尾２バイトが0x9000でなければエラー扱い
    NSUInteger length = [responseValue length];
    NSData *responseBytes = [responseValue subdataWithRange:NSMakeRange(length-2, 2)];

    char successChars[] = {0x90, 0x00};
    NSData *successBytes = [NSData dataWithBytes:successChars length:sizeof(successChars)];
    bool compare = [responseBytes isEqualToData:successBytes];

    if (compare == false) {
        [self setLastOccuredErrorMessage:@"BLEエラーが発生しました。処理を再試行してください。"];
        [self setCommandSuccess:false];
        return false;
    } else {
        [self setCommandSuccess:true];
    }

    // コマンドに応じ、以下の処理に分岐
    bool doNextCommand = false;
    switch (self.command) {
        case COMMAND_ERASE_BOND:
            NSLog(@"Erase bonding information end");
            break;
        case COMMAND_ERASE_SKEY_CERT:
            NSLog(@"Erase secure key and certificate end");
            break;
        case COMMAND_INSTALL_SKEY:
            NSLog(@"Install secure key end");
            // 後続処理のコマンドを設定
            self.command = COMMAND_INSTALL_CERT;
            doNextCommand = [self createCommandArray];
            break;
        case COMMAND_INSTALL_CERT:
            NSLog(@"Install certificate end");
            break;
        case COMMAND_CHECK_HEALTH:
            NSLog(@"Health check end");
            // 後続処理のコマンドを設定
            //self.command = FIXME_COMMAND_NAME;
            //doNextCommand = [self createCommandArray];
            break;
        default:
            break;
    }

    return doNextCommand;
}

@end
