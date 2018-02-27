//
//  ToolFileMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "ToolFileMenu.h"
#import "ToolParamWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

// OpenSSL関連処理
#include "OpenSSL.h"

@interface ToolFileMenu () <ToolParamWindowDelegate>

    @property (nonatomic) Command          command;
    @property (nonatomic) ToolParamWindow *toolParamWindow;

    - (NSString *)getProcessMessage;

@end

@implementation ToolFileMenu

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolFileMenuDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        // パラメーター入力ウィンドウを使用
        [self setToolParamWindow:[[ToolParamWindow alloc] initWithDelegate:self]];
        // OpenSSL初期化処理を実行
        init_openssl();
        return self;
    }

    - (NSString *)getProcessMessage {
        // OpenSSL処理結果のメッセージを格納
        NSString *message = [[NSString alloc] initWithBytes:get_openssl_message()
                                                     length:get_openssl_message_length()
                                                   encoding:(NSUTF8StringEncoding)];
        NSLog(@"%s", get_openssl_message());
        return message;
    }

    - (bool)createKeypairPemFile {
        // パラメーターをログ出力
        KeyPairParameter *parameter = [[self toolParamWindow] keyPairParameter];
        NSString *logMessage = [NSString
                                stringWithFormat:@"createKeypairPemFile: output[%1$@]",
                                [parameter outPath]
                                ];
        NSLog(@"%@", logMessage);
        [self.delegate notifyToolFileMenuMessage:logMessage];
        
        // 指定のパスに、EC鍵ファイルをPEM形式で生成
        return create_keypair_pem_file([[parameter outPath] UTF8String]);
    }

    - (bool)createCertreqCsrFile {
        // パラメーターをログ出力
        CertReqParameter *parameter = [[self toolParamWindow] certReqParameter];
        NSString *logMessage = [NSString
                                stringWithFormat:@"createCertreqCsrFile: CN[%1$@] OU[%2$@] O[%3$@] L[%4$@] ST[%5$@] C[%6$@] keyfile[%7$@] --> output[%8$@]",
                                [parameter CN],
                                [parameter OU],
                                [parameter O],
                                [parameter L],
                                [parameter ST],
                                [parameter C],
                                [parameter pemPath],
                                [parameter outPath]
                                ];
        NSLog(@"%@", logMessage);
        [self.delegate notifyToolFileMenuMessage:logMessage];

        // 指定のパスに、証明書要求ファイルをPEM形式で生成
        return create_certreq_csr_file([[parameter outPath] UTF8String]);
    }

    - (bool)createSelfcrtCrtFile {
        // パラメーターをログ出力
        SelfCertParameter *parameter = [[self toolParamWindow] selfCertParameter];
        NSString *logMessage = [NSString
                                stringWithFormat:@"createSelfcrtCrtFile: csrfile[%1$@] --> output[%2$@]",
                                [parameter csrPath],
                                [parameter outPath]
                                ];
        NSLog(@"%@", logMessage);
        [self.delegate notifyToolFileMenuMessage:logMessage];
        
        // 指定のパスに、自己署名証明書ファイルをDER形式で生成
        return create_selfcrt_crt_file([[parameter outPath] UTF8String]);
    }

- (void)toolFileMenuWillCreateFile:(id)sender parentWindow:(NSWindow *)parentWindow command:(Command)command {
        // コマンドに応じ、以下の処理に分岐
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                // ファイル保存パネルをモーダル表示（親画面＝メインウィンドウ）
                [[self toolParamWindow] keypairParamWindowWillSetup:sender parentWindow:parentWindow];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                // 証明書要求ファイル作成ダイアログをモーダル表示
                [[self toolParamWindow] certreqParamWindowWillSetup:sender parentWindow:parentWindow];
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                // 自己署名証明書ファイル作成ダイアログをモーダル表示
                [[self toolParamWindow] selfcrtParamWindowWillSetup:sender parentWindow:parentWindow];
                break;
            default:
                break;
        }
    }

#pragma mark - Call back from ToolParamWindow

    - (void)keypairParamWindowDidSetup:(id)sender {
        // 秘密鍵ファイル作成処理を実行
        [self processCommand:sender];
    }

    - (void)certreqParamWindowDidSetup:(id)sender {
        // 証明書要求ファイル作成処理を実行
        [self processCommand:sender];
    }

    - (void)selfcrtParamWindowDidSetup:(id)sender {
        // 自己署名証明書ファイル作成処理を実行
        [self processCommand:sender];
    }

#pragma mark - Main process

    - (void)processCommand:(id)sender {
        // コマンドに応じ、以下の処理に分岐
        bool ret = false;
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                ret = [self createKeypairPemFile];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                ret = [self createCertreqCsrFile];
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                ret = [self createSelfcrtCrtFile];
                break;
            default:
                break;
        }
        
        // 処理結果メッセージを画面表示
        [self.delegate notifyToolFileMenuMessage:[self getProcessMessage]];

        // 処理終了メッセージを、テキストエリアとポップアップの両方に表示させる
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         [self processNameOfCommand],
                         ret? MSG_SUCCESS:MSG_FAILURE];
        [[self delegate] notifyToolFileMenuMessage:str];
        if (ret) {
            [ToolPopupWindow informational:str informativeText:nil];
        } else {
            [ToolPopupWindow critical:str informativeText:nil];
        }
        
        // 処理終了をAppDelegateに通知
        [self.delegate notifyToolFileMenuEnd];
    }

    - (NSString *)processNameOfCommand {
        // 現在実行中のコマンドに対応する名称を戻す
        NSString *processName;
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                processName = PROCESS_NAME_CREATE_KEYPAIR;
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                processName = PROCESS_NAME_CREATE_CERTREQ;
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                processName = PROCESS_NAME_CREATE_SELFCRT;
                break;
            default:
                processName = nil;
                break;
        }
        return processName;
    }

@end
