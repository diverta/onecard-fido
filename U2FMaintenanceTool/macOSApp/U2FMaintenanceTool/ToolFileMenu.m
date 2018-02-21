//
//  ToolFileMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "ToolFileMenu.h"

#include "OpenSSL.h"

@interface ToolFileMenu ()

    @property (nonatomic) NSString *outputFilePath;
    @property (nonatomic) Command   command;

    - (NSString *)getProcessMessage;

    - (bool)createKeypairPemFile;
    - (bool)createCertreqCsrFile;
    - (bool)createSelfcrtCrtFile;

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

- (void)toolCommandWillCreateFile:(Command)command {
    // 処理結果を保持
    bool ret = false;
    
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
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

#pragma mark Class member accessor

- (void)setupCommand:(Command)command {
    [self setCommand:command];
}

- (Command)getCommand {
    return [self command];
}

- (void)setupOutputFilePath:(NSString *)outputFilePath {
    [self setOutputFilePath:outputFilePath];
}

@end
