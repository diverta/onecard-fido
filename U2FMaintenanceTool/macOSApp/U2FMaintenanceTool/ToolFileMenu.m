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
        // 指定のパスに、証明書要求ファイルをPEM形式で生成
        return create_certreq_csr_file([outputFilePath UTF8String]);
    }

    - (bool)createSelfcrtCrtFile:(NSString *)outputFilePath {
        // 指定のパスに、自己署名証明書ファイルをDER形式で生成
        return create_selfcrt_crt_file([outputFilePath UTF8String]);
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
                [[self toolFilePanel] prepareSavePanel:@"作成" message:@"作成する秘密鍵ファイル(PEM)名を指定してください"
                                              fileName:@"U2FPrivKey" fileTypes:@[@"pem"]];
                [[self toolFilePanel] panelWillCreatePath:appDelegate parentWindow:[appDelegate window]];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                // 証明書要求ファイル作成ダイアログをモーダル表示
                [[self toolParamWindow] prepareCertreqParamWindow];
                [[self toolParamWindow] certreqParamWindowWillSetup:appDelegate];
                break;
            default:
                break;
        }
    }

#pragma mark - Call back from ToolParamWindow

    - (void)certreqParamWindowDidSetup:(id)sender {
        // 仮コード
        NSLog(@"certreqParamWindowDidSetup called.");
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath {
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath {
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
                ret = [self createSelfcrtCrtFile:filePath];
                break;
            default:
                break;
        }
        
        if (ret) {
            // 処理成功時
            [self.delegate notifyToolCommandMessage:[self getProcessMessage]];
            [self.delegate toolCommandDidSuccess];
            
        } else {
            // 処理失敗時
            [self.delegate toolCommandDidFail:[self getProcessMessage]];
        }
    }

    - (NSString *)processNameOfCommand {
        // 現在実行中のコマンドに対応する名称を戻す
        NSString *processName;
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                processName = @"鍵ファイル作成";
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                processName = @"証明書要求ファイル作成";
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                processName = @"自己署名証明書ファイル作成";
                break;
            default:
                processName = nil;
                break;
        }
        return processName;
    }

@end
