//
//  ToolFileMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "AppDelegate.h"
#import "ToolFileMenu.h"
#import "ToolFilePanel.h"
#import "ToolParamWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

// OpenSSL関連処理
#include "OpenSSL.h"

@interface ToolFileMenu () <ToolFilePanelDelegate, ToolParamWindowDelegate>

    @property (nonatomic) Command          command;
    @property (nonatomic) ToolFilePanel   *toolFilePanel;
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
        // ファイル保存／選択パネル、パラメーター入力ウィンドウを使用
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
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

    - (bool)createKeypairPemFile:(NSString *)outputFilePath {
        // 指定のパスに、EC鍵ファイルをPEM形式で生成
        return create_keypair_pem_file([outputFilePath UTF8String]);
    }

    - (bool)createCertreqCsrFile:(NSString *)outputFilePath {
        // パラメーターをログ出力
        NSString *logMessage = [NSString
                                stringWithFormat:@"createCertreqCsrFile: CN[%1$@] OU[%2$@] O[%3$@] L[%4$@] ST[%5$@] C[%6$@] keyfile[%7$@] --> output[%8$@]",
                                [[self toolParamWindow] certreqParamCN],
                                [[self toolParamWindow] certreqParamOU],
                                [[self toolParamWindow] certreqParamO],
                                [[self toolParamWindow] certreqParamL],
                                [[self toolParamWindow] certreqParamST],
                                [[self toolParamWindow] certreqParamC],
                                [[self toolParamWindow] certreqParamPemPath],
                                outputFilePath
                                ];
        NSLog(@"%@", logMessage);
        [self.delegate notifyToolFileMenuMessage:logMessage];

        // 指定のパスに、証明書要求ファイルをPEM形式で生成
        return create_certreq_csr_file([outputFilePath UTF8String]);
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

    - (void)toolFileMenuWillCreateFile:(id)sender {
        // メニューアイテムからコマンドを判定
        AppDelegate *appDelegate = (AppDelegate *)[self delegate];
        if (sender == [appDelegate menuItemFile1]) {
            [self setCommand:COMMAND_CREATE_KEYPAIR_PEM];
            
        } else if (sender == [appDelegate menuItemFile2]) {
            [self setCommand:COMMAND_CREATE_CERTREQ_CSR];
            
        } else if (sender == [appDelegate menuItemFile3]) {
            [self setCommand:COMMAND_CREATE_SELFCRT_CRT];
        }
        
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                // ファイル保存パネルをモーダル表示（親画面＝メインウィンドウ）
                [[self toolFilePanel] prepareSavePanel:MSG_BUTTON_CREATE
                                               message:MSG_PROMPT_CREATE_PEM_PATH
                                              fileName:@"U2FPrivKey" fileTypes:@[@"pem"]];
                [[self toolFilePanel] panelWillCreatePath:appDelegate parentWindow:[appDelegate window]];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                // 証明書要求ファイル作成ダイアログをモーダル表示
                [[self toolParamWindow] certreqParamWindowWillSetup:appDelegate parentWindow:[appDelegate window]];
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                // 自己署名証明書ファイル作成ダイアログをモーダル表示
                [[self toolParamWindow] selfcrtParamWindowWillSetup:appDelegate parentWindow:[appDelegate window]];
                break;
            default:
                break;
        }
    }

#pragma mark - Call back from ToolParamWindow

    - (void)certreqParamWindowDidSetup:(id)sender {
        // 証明書要求ファイル作成処理を実行
        [self processCommand:sender filePath:[[self toolParamWindow] certreqParamOutPath]];
    }

    - (void)selfcrtParamWindowDidSetup:(id)sender {
        // 自己署名証明書ファイル作成処理を実行
        [self processCommand:sender filePath:nil];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath {
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath {
        // 秘密鍵ファイル作成処理を実行
        [self processCommand:sender filePath:filePath];
    }

#pragma mark - Main process

    - (void)processCommand:(id)sender filePath:(NSString*)filePath {
        // コマンドに応じ、以下の処理に分岐
        bool ret = false;
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                ret = [self createKeypairPemFile:filePath];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                ret = [self createCertreqCsrFile:filePath];
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
