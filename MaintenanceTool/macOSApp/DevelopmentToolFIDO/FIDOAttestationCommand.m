//
//  FIDOAttestationCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/22.
//
#import "fido_crypto.h"
#import "AppCommonMessage.h"
#import "FIDOAttestationCommand.h"
#import "FIDODefines.h"
#import "ToolLogFile.h"

@interface FIDOAttestationCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id          delegate;
    @property (nonatomic) NSMutableData    *message;

@end

@implementation FIDOAttestationCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

    - (NSData *)generatedInstallMessage {
        return [self message];
    }

    - (bool)generateInstallMessageFrom:(NSArray<NSString *> *)selectedFilePaths {
        // 鍵／証明書ファイルのパスを抽出
        NSString *skeyFilePath = [selectedFilePaths objectAtIndex:0];
        NSString *certFilePath = [selectedFilePaths objectAtIndex:1];
        
        // 鍵ファイルから秘密鍵（32バイト）を取得し、レスポンスメッセージ領域に格納
        NSData *dataSkey = [self readSkeyFromFile:skeyFilePath];
        if (dataSkey == nil) {
            return false;
        }

        // 証明書ファイルから内容を取得
        NSData *dataCert = [self readCertFromFile:certFilePath];
        if (dataCert == nil) {
            return false;
        }

        // 秘密鍵と証明書の整合性検証を行う
        if (validate_skey_cert((uint8_t *)[dataSkey bytes], [dataSkey length], (uint8_t *)[dataCert bytes], [dataCert length]) != CTAP1_ERR_SUCCESS) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Public key validation failed: %s", log_debug_message()];
            [[self delegate] notifyErrorMessage:MSG_INVALID_SKEY_OR_CERT];
            return false;
        }

        // 秘密鍵を証明書をマージ
        [self setMessage:[[NSMutableData alloc] init]];
        [[self message] appendData:dataSkey];
        [[self message] appendData:dataCert];
        return true;
    }

    #pragma mark - Read private key data

    - (NSData *)readSkeyFromFile:(NSString *)skeyFilePath {
        // 鍵ファイルから読み込み
        NSError  *err;
        NSString *str = [NSString stringWithContentsOfFile:skeyFilePath encoding:NSUTF8StringEncoding error:&err];
        if (err.code) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Secure key file read error: %@", [err description]];
            [[self delegate] notifyErrorMessage:MSG_CANNOT_READ_SKEY_PEM_FILE];
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
            [[ToolLogFile defaultLogger] error:@"Secure key file has no header 'BEGIN EC PRIVATE KEY'"];
            [[self delegate] notifyErrorMessage:MSG_INVALID_SKEY_CONTENT_IN_PEM];
            return nil;
        }

        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
        // 先頭からビッグエンディアン形式で配置
        return [self convertSkeyPem:pem];
    }

    - (NSData *)convertSkeyPem:(NSString *)skeyPemContents {
        // Base64エンコードされた秘密鍵文字列をデコード
        NSData *decodedPemData = [[NSData alloc]
              initWithBase64EncodedString:skeyPemContents
              options:NSDataBase64DecodingIgnoreUnknownCharacters];

        // デコードされたデータが39バイト未満の場合はエラー
        if ([decodedPemData length] < 39) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Secure key has invalid length: %ld", [decodedPemData length]];
            [[self delegate] notifyErrorMessage:MSG_INVALID_SKEY_LENGTH_IN_PEM];
            return nil;
        }

        // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
        const char *decodedPem = [decodedPemData bytes];
        if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]];
            [[self delegate] notifyErrorMessage:MSG_INVALID_SKEY_HEADER_IN_PEM];
            return nil;
        }

        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
        // 先頭からビッグエンディアン形式で配置
        char keyBuffer[32];
        for (int i = 0; i < 32; i++) {
            keyBuffer[i] = decodedPem[7 + i];
        }

        // NSDataに変換された秘密鍵を戻す
        NSData *skeyData = [[NSData alloc] initWithBytes:keyBuffer length:sizeof(keyBuffer)];
        return skeyData;
    }

    #pragma mark - Read certificate data

    - (NSData *)readCertFromFile:(NSString *)certFilePath {
        // 証明書ファイルから読み込み
        NSData *data = [NSData dataWithContentsOfFile:certFilePath];
        if (data == nil || [data length] == 0) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Cannot read cert file: %@", certFilePath];
            [[self delegate] notifyErrorMessage:MSG_CANNOT_READ_CERT_CRT_FILE];
            return nil;
        }

        // 証明書ファイルの長さが68バイト未満の場合はエラー
        NSUInteger dataCertLength = [data length];
        if (dataCertLength < 68) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Invalid cert length: %@", dataCertLength];
            [[self delegate] notifyErrorMessage:MSG_INVALID_CERT_LENGTH_IN_CRT];
            return nil;
        }

        return data;
    }

@end
