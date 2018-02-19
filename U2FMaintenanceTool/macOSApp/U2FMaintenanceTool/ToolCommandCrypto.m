//
//  ToolCommandCrypto.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/19.
//
#import <Foundation/Foundation.h>
#import "ToolCommandCrypto.h"

#pragma mark OpenSSL関連処理群

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>

static char openssl_message[1024];

const char *get_openssl_message(void) {
    return openssl_message;
}

size_t get_openssl_message_length(void) {
    return strlen(openssl_message);
}

void init_openssl(void) {
    // OpenSSLの実行に必要な初期化処理を実行
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

void free_resources(EC_KEY *eckey, EVP_PKEY *pkey, FILE *output_file) {
    // 作業領域を解放し、ファイルを閉じる
    EVP_PKEY_free(pkey);
    EC_KEY_free(eckey);
    if (output_file) {
        fclose(output_file);
    }
}

bool create_keypair_pem_file(const char *output_file_path) {
    // secp256r1という定義がないので、同義のprime256v1を指定
    int eccgrp = OBJ_txt2nid(SN_X9_62_prime256v1);
    EC_KEY *eckey = EC_KEY_new_by_curve_name(eccgrp);
    if (eckey == NULL) {
        sprintf(openssl_message, "EC_KEY_new_by_curve_name failed");
        return false;
    }

    // 新規のキーペアを生成
    EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
    if (EC_KEY_generate_key(eckey) == 0) {
        sprintf(openssl_message, "EC_KEY_generate_key failed");
        free_resources(eckey, NULL, NULL);
        return false;
    }
    
    // EC鍵の格納領域を生成
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (EVP_PKEY_assign_EC_KEY(pkey, eckey) == 0) {
        sprintf(openssl_message, "EVP_PKEY_assign_EC_KEY failed");
        free_resources(eckey, pkey, NULL);
        return false;
    }
    
    // EC鍵を生成
    eckey = EVP_PKEY_get1_EC_KEY(pkey);
    if (eckey == NULL) {
        sprintf(openssl_message, "EVP_PKEY_get1_EC_KEY failed");
        free_resources(eckey, pkey, NULL);
        return false;
    }
    
    // EC鍵ファイルを開く
    FILE *fp = fopen(output_file_path, "w");
    if (fp == NULL) {
        sprintf(openssl_message, "fopen failed: %s", output_file_path);
        free_resources(eckey, pkey, fp);
        return false;
    }
    
    // EC鍵をファイルに書き出し
    if (PEM_write_ECPrivateKey(fp, eckey, NULL, NULL, 0, 0, NULL) == 0) {
        sprintf(openssl_message, "PEM_write_ECPrivateKey failed");
        free_resources(eckey, pkey, fp);
        return false;
    }
    
    // 正常終了
    sprintf(openssl_message, "鍵ファイルを作成しました: %s", output_file_path);
    free_resources(eckey, pkey, fp);
    return true;
}

#pragma mark OpenSSL関連処理を実行するためのクラス

@implementation ToolCommandCrypto

- (id)init {
    // OpenSSL初期化処理を実行
    init_openssl();
    return self;
}

- (void)setOpenSSLMessage {
    // OpenSSL処理結果のメッセージを格納
    NSString *message = [NSString alloc];
    [self setProcessMessage:[message initWithBytes:get_openssl_message()
                                            length:get_openssl_message_length()
                                          encoding:(NSUTF8StringEncoding)]];
}

- (bool)createKeypairPemFile {
    // 指定のパスに、EC鍵ファイルをPEM形式で生成
    bool ret = create_keypair_pem_file([[self outputFilePath] UTF8String]);
    
    // メッセージを出力
    [self setOpenSSLMessage];
    NSLog(@"createKeypairPemFile %@: %s", ret ? @"success" : @"failed", get_openssl_message());
    return ret;
}

@end
