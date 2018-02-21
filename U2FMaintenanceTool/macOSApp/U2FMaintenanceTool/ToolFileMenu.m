//
//  ToolFileMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "ToolFileMenu.h"

#include "OpenSSL.h"

@implementation ToolFileMenu

- (id)init {
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

@end
