//
//  ToolFileMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "ToolFileMenu.h"
#import "AppDelegate.h"

// 個別実装ダイアログ
#import "CertreqParamWindow.h"

// OpenSSL関連処理
#include "OpenSSL.h"

@interface ToolFileMenu ()

    @property (nonatomic) NSString *outputFilePath;
    @property (nonatomic) Command   command;

    @property (nonatomic) CertreqParamWindow *certreqParamWindow;

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
        // 指定のパスに、EC鍵ファイルをPEM形式で生成
        return create_keypair_pem_file([[self outputFilePath] UTF8String]);
    }

    - (bool)createCertreqCsrFile {
        // 指定のパスに、証明書要求ファイルをPEM形式で生成
        return create_certreq_csr_file([[self outputFilePath] UTF8String]);
    }

    - (bool)createSelfcrtCrtFile {
        // 指定のパスに、自己署名証明書ファイルをDER形式で生成
        return create_selfcrt_crt_file([[self outputFilePath] UTF8String]);
    }

    - (NSSavePanel *)preparePanelForCreatePath {
        // ファイル保存パネルの設定
        NSSavePanel *panel = [NSSavePanel savePanel];
        [panel setCanCreateDirectories:NO];
        [panel setShowsTagField:NO];
        // コマンド種別ごとに設定値を変える
        switch ([self command]) {
            case COMMAND_CREATE_KEYPAIR_PEM:
                [panel setMessage:@"作成する秘密鍵ファイル(PEM)名を指定してください"];
                [panel setNameFieldStringValue:@"U2FPrivKey"];
                [panel setAllowedFileTypes:@[@"pem"]];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                [panel setMessage:@"作成する証明書要求ファイル(CSR)名を指定してください"];
                [panel setNameFieldStringValue:@"U2FCertReq"];
                [panel setAllowedFileTypes:@[@"csr"]];
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                [panel setMessage:@"作成する自己署名証明書ファイル(CRT)名を指定してください"];
                [panel setNameFieldStringValue:@"U2FSelfCer"];
                [panel setAllowedFileTypes:@[@"crt"]];
                break;
            default:
                break;
        }
        [panel setPrompt:@"作成"];
        
        return panel;
    }

    - (NSWindow *)prepareKeypairPemWindow {
        if ([self certreqParamWindow] == nil) {
            [self setCertreqParamWindow:[[CertreqParamWindow alloc]
                                         initWithWindowNibName:@"CertreqParamWindow"]];
        }
        if ([self certreqParamWindow]) {
            return [[self certreqParamWindow] window];
        }
        return nil;
    }

    - (void)displayParamWindowAsDialog:(NSWindow *)window parent:(id)parent {
        if (window == nil) {
            return;
        }
        // ダイアログをモーダルで表示
        [NSApp runModalForWindow:window];
        [NSApp endSheet:window];
        // 親のウィンドウを表示させないようにする
        [window orderOut:parent];
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
                // ファイル保存パネルをモーダル表示
                [[self delegate] panelWillCreatePath:[self preparePanelForCreatePath] sender:self];
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                // ダイアログを表示
                [self displayParamWindowAsDialog:[self prepareKeypairPemWindow] parent:appDelegate];
                break;
            default:
                break;
        }
    }

    - (bool)panelDidCreatePath:(NSString*)filePath {
        // 作成された出力先パスを保持
        [self setOutputFilePath:filePath];
        
        // 処理結果を保持
        bool ret = false;
        
        // コマンドに応じ、以下の処理に分岐
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
        
        if (ret) {
            // 処理成功時
            [self.delegate notifyToolCommandMessage:[self getProcessMessage]];
            [self.delegate toolCommandDidSuccess];
            
        } else {
            // 処理失敗時
            [self.delegate toolCommandDidFail:[self getProcessMessage]];
        }
        return ret;
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
