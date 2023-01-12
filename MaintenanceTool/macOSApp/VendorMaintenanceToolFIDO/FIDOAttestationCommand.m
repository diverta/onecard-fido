//
//  FIDOAttestationCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "debug_log.h"
#import "fido_crypto.h"
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "FIDOAttestationCommand.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "VendorFunctionCommand.h"

@interface FIDOAttestationCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                          delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                    *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                           command;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType                     transportType;
    // 処理のパラメーターを保持
    @property (nonatomic) VendorFunctionCommandParameter   *commandParameter;
    // 下位関数のエラーメッセージを保持
    @property (nonatomic) NSString                         *errorMessage;

@end

@implementation FIDOAttestationCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)doRequestInstallAttestation:(id)commandParameterRef {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // パラメーターを保持
        [self setCommandParameter:(VendorFunctionCommandParameter *)commandParameterRef];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_INSTALL_ATTESTATION];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doRequestRemoveAttestation {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_REMOVE_ATTESTATION];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        if ([self command] == COMMAND_INSTALL_ATTESTATION) {
            [self doRequestHidInstallAttestation];
        }
        if ([self command] == COMMAND_REMOVE_ATTESTATION) {
            [self doRequestHidRemoveAttestation];
        }
    }

    - (void)doRequestHidInstallAttestation {
        // メッセージを編集し、コマンド 0xC8 を実行
        NSMutableData *data = [[NSMutableData alloc] init];
        NSArray<NSString *> *pemFiles = @[[[self commandParameter] pkeyPemPath], [[self commandParameter] certPemPath]];
        if ([self generateRequestDataForInstall:data fromPemFiles:pemFiles] == false) {
            [self commandDidProcess:false message:[self errorMessage]];
            return;
        }
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_INSTALL_ATTESTATION withCMD:HID_CMD_INSTALL_ATTESTATION withData:data];
    }

    - (void)doResponseHidInstallAttestation:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            NSString *message = [NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_ST, responseBytes[0]];
            [self commandDidProcess:false message:message];
            return;
        }
        [self commandDidProcess:true message:nil];
    }

    - (void)doRequestHidRemoveAttestation {
        // メッセージを編集し、コマンド 0xC9 を実行
        NSData *data = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_REMOVE_ATTESTATION withCMD:HID_CMD_RESET_ATTESTATION withData:data];
    }

    - (void)doResponseHidRemoveAttestation:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            NSString *message = [NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_ST, responseBytes[0]];
            [self commandDidProcess:false message:message];
            return;
        }
        [self commandDidProcess:true message:nil];
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] FIDOAttestationCommandDidCompleted:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        if ([self command] == COMMAND_INSTALL_ATTESTATION || [self command] == COMMAND_REMOVE_ATTESTATION) {
            if (success == false) {
                // 即時で上位クラスに制御を戻す
                [self commandDidProcess:false message:errorMessage];
                return;
            }
            // 実行コマンドにより処理分岐
            if (command == COMMAND_HID_CTAP2_INIT) {
                [self doResponseHIDCtap2Init];
            }
            if (command == COMMAND_INSTALL_ATTESTATION) {
                [self doResponseHidInstallAttestation:response];
            }
            if (command == COMMAND_REMOVE_ATTESTATION) {
                [self doResponseHidRemoveAttestation:response];
            }
        }
    }

#pragma mark - Generate request data for install

    - (bool)generateRequestDataForInstall:(NSMutableData *)requestData fromPemFiles:(NSArray<NSString *> *)pemFiles {
        // 鍵／証明書ファイルのパスを抽出
        NSString *skeyFilePath = [pemFiles objectAtIndex:0];
        NSString *certFilePath = [pemFiles objectAtIndex:1];
        
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
            [self setErrorMessage:MSG_INVALID_SKEY_OR_CERT];
            return false;
        }

        // 秘密鍵を証明書をマージ
        [requestData appendData:dataSkey];
        [requestData appendData:dataCert];
        return true;
    }

    #pragma mark - Read private key data

    - (NSData *)readSkeyFromFile:(NSString *)skeyFilePath {
        // 鍵ファイルから読み込み
        NSError  *err;
        NSString *str = [NSString stringWithContentsOfFile:skeyFilePath encoding:NSUTF8StringEncoding error:&err];
        if (err.code) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Secure key file read error: %@", [err description]];
            [self setErrorMessage:MSG_CANNOT_READ_SKEY_PEM_FILE];
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
            [self setErrorMessage:MSG_INVALID_SKEY_CONTENT_IN_PEM];
            return nil;
        }

        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、先頭からビッグエンディアン形式で配置
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
            [self setErrorMessage:MSG_INVALID_SKEY_LENGTH_IN_PEM];
            return nil;
        }

        // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
        const char *decodedPem = [decodedPemData bytes];
        if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]];
            [self setErrorMessage:MSG_INVALID_SKEY_HEADER_IN_PEM];
            return nil;
        }

        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、先頭からビッグエンディアン形式で配置
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
            [self setErrorMessage:MSG_CANNOT_READ_CERT_CRT_FILE];
            return nil;
        }

        // 証明書ファイルの長さが68バイト未満の場合はエラー
        NSUInteger dataCertLength = [data length];
        if (dataCertLength < 68) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Invalid cert length: %@", dataCertLength];
            [self setErrorMessage:MSG_INVALID_CERT_LENGTH_IN_CRT];
            return nil;
        }

        return data;
    }

@end
