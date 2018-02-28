//
//  OpenSSL.c
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/21.
//
#include "OpenSSL.h"

#include <string.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>

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

static void free_resources(EC_KEY *eckey, EVP_PKEY *pkey, FILE *output_file) {
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
        sprintf(openssl_message, "create_keypair_pem_file: EC_KEY_new_by_curve_name failed");
        return false;
    }
    
    // 新規のキーペアを生成
    EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
    if (EC_KEY_generate_key(eckey) == 0) {
        sprintf(openssl_message, "create_keypair_pem_file: EC_KEY_generate_key failed");
        free_resources(eckey, NULL, NULL);
        return false;
    }
    
    // EC鍵の格納領域を生成
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (EVP_PKEY_assign_EC_KEY(pkey, eckey) == 0) {
        sprintf(openssl_message, "create_keypair_pem_file: EVP_PKEY_assign_EC_KEY failed");
        free_resources(eckey, pkey, NULL);
        return false;
    }
    
    // EC鍵を生成
    eckey = EVP_PKEY_get1_EC_KEY(pkey);
    if (eckey == NULL) {
        sprintf(openssl_message, "create_keypair_pem_file: EVP_PKEY_get1_EC_KEY failed");
        free_resources(eckey, pkey, NULL);
        return false;
    }
    
    // EC鍵ファイルを開く
    FILE *fp = fopen(output_file_path, "w");
    if (fp == NULL) {
        sprintf(openssl_message, "create_keypair_pem_file: fopen failed: %s", output_file_path);
        free_resources(eckey, pkey, fp);
        return false;
    }
    
    // EC鍵をファイルに書き出し
    if (PEM_write_ECPrivateKey(fp, eckey, NULL, NULL, 0, 0, NULL) == 0) {
        sprintf(openssl_message, "create_keypair_pem_file: PEM_write_ECPrivateKey failed");
        free_resources(eckey, pkey, fp);
        return false;
    }
    
    // 正常終了
    sprintf(openssl_message, "create_keypair_pem_file: EC private key file created successfully.");
    free_resources(eckey, pkey, fp);
    return true;
}

static void free_resources_for_csr(FILE *input_file, EVP_PKEY *pkey, EC_KEY *eckey,
                                   X509_REQ *x509_req, FILE *output_file) {
    // 作業領域を解放し、ファイルを閉じる
    if (input_file) {
        fclose(input_file);
    }
    if (output_file) {
        fclose(output_file);
    }
    EC_KEY_free(eckey);
    EVP_PKEY_free(pkey);
    X509_REQ_free(x509_req);
}

static bool X509_NAME_add_entry_for_csr(X509_NAME *x509_name, const char *param_name, const char *param_value) {
    if (X509_NAME_add_entry_by_txt(x509_name, param_name, MBSTRING_ASC,
                                   (const unsigned char*)param_value, -1, -1, 0) == 0) {
        sprintf(openssl_message,
                "create_certreq_csr_file: X509_NAME_add_entry_by_txt[%s=%s] failed",
                param_name, param_value);
        return false;
    }
    return true;
}

static bool X509_NAME_add_entries_for_csr(X509_NAME *x509_name,
    const char *CN, const char *OU, const char *O, const char *L, const char *ST, const char *C) {
    if (X509_NAME_add_entry_for_csr(x509_name, "CN", CN) == false) {
        return false;
    }
    if (strlen(OU) > 0) {
        if (X509_NAME_add_entry_for_csr(x509_name, "OU", OU) == false) {
            return false;
        }
    }
    if (X509_NAME_add_entry_for_csr(x509_name, "O", O) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(x509_name, "L", L) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(x509_name, "ST", ST) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(x509_name, "C", C) == false) {
        return false;
    }
    return true;
}

bool add_x509_v3_extension(X509_REQ *x509_req) {
    // BLEトランスポートサポートに関する拡張属性を付与（bluetoothLowEnergyRadio）
    STACK_OF(X509_EXTENSION) *exts = sk_X509_EXTENSION_new_null();
    int nid = OBJ_create("1.3.6.1.4.1.45724.2.1.1",
                         "U2F Extension",
                         "FIDO U2F Authenticator Transports Extension");
    const char *value = "DER:03:02:06:40";
    
    X509_EXTENSION *ex = X509V3_EXT_conf_nid(NULL, NULL, nid, value);
    if (ex == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: X509V3_EXT_conf_nid failed");
        return false;
    }
    
    sk_X509_EXTENSION_push(exts, ex);
    X509_REQ_add_extensions(x509_req, exts);
    
    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    return true;
}

bool create_certreq_csr_file(const char *output_file_path, const char *privkey_file_path,
    const char *CN, const char *OU, const char *O, const char *L, const char *ST, const char *C) {
    // EC鍵ファイルを開く
    FILE *fp = fopen(privkey_file_path, "r");
    if (fp == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: fopen failed: %s", privkey_file_path);
        return false;
    }

    // EC鍵ファイルからEC鍵を取得
    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    if (pkey == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: PEM_read_PrivateKey failed");
        free_resources_for_csr(fp, pkey, NULL, NULL, NULL);
        return false;
    }
    EC_KEY *eckey = EVP_PKEY_get1_EC_KEY(pkey);
    if (eckey == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: EVP_PKEY_get1_EC_KEY failed");
        free_resources_for_csr(fp, pkey, eckey, NULL, NULL);
        return false;
    }
    
    // CSRの格納領域を生成
    int n_version = 0;
    X509_REQ *x509_req = X509_REQ_new();
    if (X509_REQ_set_version(x509_req, n_version) == 0) {
        sprintf(openssl_message, "create_certreq_csr_file: X509_REQ_set_version failed");
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }
    
    // CSRに各項目の値を設定
    X509_NAME *x509_name = X509_REQ_get_subject_name(x509_req);
    if (x509_name == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: X509_REQ_get_subject_name failed");
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }
    if (X509_NAME_add_entries_for_csr(x509_name, CN, OU, O, L, ST, C) == false) {
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }
    
    // BLEトランスポートサポートに関する拡張属性を付与
    if (add_x509_v3_extension(x509_req) == false) {
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }
    
    // CSRに公開鍵を設定
    if (X509_REQ_set_pubkey(x509_req, pkey) == 0) {
        sprintf(openssl_message, "create_certreq_csr_file: X509_REQ_set_pubkey failed");
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }

    // CSRに署名
    if (X509_REQ_sign(x509_req, pkey, EVP_sha1()) < 1) {
        sprintf(openssl_message, "create_certreq_csr_file: X509_REQ_sign failed");
        free_resources_for_csr(fp, pkey, eckey, x509_req, NULL);
        return false;
    }
    
    // CSRファイルを開く
    FILE *outfp = fopen(output_file_path, "w");
    if (outfp == NULL) {
        sprintf(openssl_message, "create_certreq_csr_file: fopen failed: %s", output_file_path);
        free_resources_for_csr(fp, pkey, eckey, x509_req, outfp);
        return false;
    }
    
    // CSRをファイルに書き出し
    if (PEM_write_X509_REQ(outfp, x509_req) == 0) {
        sprintf(openssl_message, "create_certreq_csr_file: PEM_write_X509_REQ failed");
        free_resources_for_csr(fp, pkey, eckey, x509_req, outfp);
        return false;
    }

    sprintf(openssl_message, "create_certreq_csr_file: CSR file created successfully.");
    free_resources_for_csr(fp, pkey, eckey, x509_req, outfp);
    return true;
}

bool create_selfcrt_crt_file(const char *output_file_path) {
    sprintf(openssl_message, "create_selfcrt_crt_file: under construction");
    return false;
}
