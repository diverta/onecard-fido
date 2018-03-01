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
#include <openssl/asn1.h>
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

EC_KEY *create_eckey_new(const char *function_name) {
    // secp256r1という定義がないので、同義のprime256v1を指定
    int eccgrp = OBJ_txt2nid(SN_X9_62_prime256v1);
    EC_KEY *eckey = EC_KEY_new_by_curve_name(eccgrp);
    if (eckey == NULL) {
        sprintf(openssl_message, "%s: EC_KEY_new_by_curve_name failed", function_name);
        return NULL;
    }
    // 新規のキーペアを生成
    EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
    if (EC_KEY_generate_key(eckey) == 0) {
        sprintf(openssl_message, "%s: EC_KEY_generate_key failed", function_name);
        EC_KEY_free(eckey);
        return NULL;
    }
    return eckey;
}

int keypair_pem_file_write(const char *function_name, EC_KEY *eckey, const char *output_file_path) {
    // EC鍵ファイルを開く
    FILE *outfp = fopen(output_file_path, "w");
    if (outfp == NULL) {
        sprintf(openssl_message, "%s: fopen failed: %s", function_name, output_file_path);
        return 0;
    }
    // EC鍵をファイルに書き出し
    int ret = PEM_write_ECPrivateKey(outfp, eckey, NULL, NULL, 0, 0, NULL);
    if (ret == 0) {
        sprintf(openssl_message, "%s: PEM_write_ECPrivateKey failed", function_name);
    }
    // ファイルを閉じ、リソースを解放
    fclose(outfp);
    EC_KEY_free(eckey);
    return ret;
}

bool create_keypair_pem_file(const char *output_file_path) {
    const char *function_name = "create_keypair_pem_file";
    // 新規のキーペアを生成
    EC_KEY *eckey = create_eckey_new(function_name);
    if (eckey == NULL) {
        return false;
    }
    // EC鍵をファイルに書き出し
    if (keypair_pem_file_write(function_name, eckey, output_file_path) == 0) {
        return false;
    }
    // 正常終了
    sprintf(openssl_message, "%s: EC private key file created successfully.", function_name);
    return true;
}

static void free_resources_for_csr(X509_REQ *x509_req, EVP_PKEY *pkey) {
    // 作業領域を解放
    EVP_PKEY_free(pkey);
    X509_REQ_free(x509_req);
}

static bool X509_NAME_add_entry_for_csr(const char *function_name, X509_NAME *x509_name,
                                        const char *param_name, const char *param_value) {
    if (X509_NAME_add_entry_by_txt(x509_name, param_name, MBSTRING_ASC,
                                   (const unsigned char*)param_value, -1, -1, 0) == 0) {
        sprintf(openssl_message,
                "%s: X509_NAME_add_entry_by_txt[%s=%s] failed",
                function_name, param_name, param_value);
        return false;
    }
    return true;
}

static bool X509_NAME_add_entries_for_csr(const char *function_name, X509_NAME *x509_name,
    const char *CN, const char *OU, const char *O, const char *L, const char *ST, const char *C) {
    if (X509_NAME_add_entry_for_csr(function_name, x509_name, "CN", CN) == false) {
        return false;
    }
    if (strlen(OU) > 0) {
        if (X509_NAME_add_entry_for_csr(function_name, x509_name, "OU", OU) == false) {
            return false;
        }
    }
    if (X509_NAME_add_entry_for_csr(function_name, x509_name, "O", O) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(function_name, x509_name, "L", L) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(function_name, x509_name, "ST", ST) == false) {
        return false;
    }
    if (X509_NAME_add_entry_for_csr(function_name, x509_name, "C", C) == false) {
        return false;
    }
    return true;
}

#define U2F_TRANS_EXT_BLE_VALUE "DER:03:02:06:40"

static int get_u2f_trans_ext_ble_oid(void) {
    // BLEトランスポートサポートに関する拡張属性を付与（bluetoothLowEnergyRadio）
    int nid = OBJ_create("1.3.6.1.4.1.45724.2.1.1",
                         "U2F Extension",
                         "FIDO U2F Authenticator Transports Extension");
    return nid;
}

static bool add_x509_v3_extension(const char *function_name, X509_REQ *x509_req) {
    // BLEトランスポートサポートに関する拡張属性を付与
    X509_EXTENSION *ex = X509V3_EXT_conf_nid(NULL, NULL,
                                             get_u2f_trans_ext_ble_oid(),
                                             U2F_TRANS_EXT_BLE_VALUE);
    if (ex == NULL) {
        sprintf(openssl_message, "%s: X509V3_EXT_conf_nid failed", function_name);
        return false;
    }
    // 証明書要求に拡張属性を設定
    STACK_OF(X509_EXTENSION) *exts = sk_X509_EXTENSION_new_null();
    sk_X509_EXTENSION_push(exts, ex);
    X509_REQ_add_extensions(x509_req, exts);
    // リソースを解放
    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    X509_EXTENSION_free(ex);
    return true;
}

EVP_PKEY *read_private_key(const char *function_name, const char *privkey_file_path) {
    // EC鍵ファイルを開く
    FILE *pkeyfp = fopen(privkey_file_path, "r");
    if (pkeyfp == NULL) {
        sprintf(openssl_message, "%s: fopen failed: %s", function_name, privkey_file_path);
        return NULL;
    }
    // EC鍵ファイルの内容を取得
    EVP_PKEY *pkey = PEM_read_PrivateKey(pkeyfp, NULL, NULL, NULL);
    if (pkey == NULL) {
        sprintf(openssl_message, "%s: PEM_read_PrivateKey failed", function_name);
    }
    // ファイルを閉じる
    fclose(pkeyfp);
    return pkey;
}

int x509_req_write(const char *function_name, X509_REQ *x509_req, const char *output_file_path) {
    // CSRファイルを開く
    FILE *outfp = fopen(output_file_path, "w");
    if (outfp == NULL) {
        sprintf(openssl_message, "%s: fopen failed: %s", function_name, output_file_path);
        return 0;
    }
    // CSRをファイルに書き出し
    int ret = PEM_write_X509_REQ(outfp, x509_req);
    if (ret == 0) {
        sprintf(openssl_message, "%s: PEM_write_X509_REQ failed", function_name);
    }
    // ファイルを閉じる
    fclose(outfp);
    return ret;
}

bool create_certreq_csr_file(const char *output_file_path, const char *privkey_file_path,
    const char *CN, const char *OU, const char *O, const char *L, const char *ST, const char *C) {
    const char *function_name = "create_certreq_csr_file";
    // CSRの格納領域を生成
    int n_version = 0;
    X509_REQ *x509_req = X509_REQ_new();
    if (X509_REQ_set_version(x509_req, n_version) == 0) {
        sprintf(openssl_message, "%s: X509_REQ_set_version failed", function_name);
        free_resources_for_csr(x509_req, NULL);
        return false;
    }
    // CSRに各項目の値を設定
    X509_NAME *x509_name = X509_REQ_get_subject_name(x509_req);
    if (x509_name == NULL) {
        sprintf(openssl_message, "%s: X509_REQ_get_subject_name failed", function_name);
        free_resources_for_csr(x509_req, NULL);
        return false;
    }
    if (X509_NAME_add_entries_for_csr(function_name, x509_name, CN, OU, O, L, ST, C) == false) {
        free_resources_for_csr(x509_req, NULL);
        return false;
    }
    // BLEトランスポートサポートに関する拡張属性を付与
    if (add_x509_v3_extension(function_name, x509_req) == false) {
        free_resources_for_csr(x509_req, NULL);
        return false;
    }
    // EC鍵ファイルの内容を取得
    EVP_PKEY *pkey = read_private_key(function_name, privkey_file_path);
    if (pkey == NULL) {
        free_resources_for_csr(x509_req, pkey);
        return false;
    }
    // CSRに公開鍵を設定
    if (X509_REQ_set_pubkey(x509_req, pkey) == 0) {
        sprintf(openssl_message, "%s: X509_REQ_set_pubkey failed", function_name);
        free_resources_for_csr(x509_req, pkey);
        return false;
    }
    // CSRに署名
    if (X509_REQ_sign(x509_req, pkey, EVP_sha1()) < 1) {
        sprintf(openssl_message, "%s: X509_REQ_sign failed", function_name);
        free_resources_for_csr(x509_req, pkey);
        return false;
    }
    // 証明書をファイルに書き出し
    if (x509_req_write(function_name, x509_req, output_file_path) == 0) {
        free_resources_for_csr(x509_req, pkey);
        return false;
    }
    // リソースを解放
    sprintf(openssl_message, "%s: CSR file created successfully.", function_name);
    free_resources_for_csr(x509_req, pkey);
    return true;
}

static void free_resources_for_crt(X509 *x509_cert, X509_REQ *x509_req, EVP_PKEY *pkey) {
    // 作業領域を解放
    X509_free(x509_cert);
    X509_REQ_free(x509_req);
    EVP_PKEY_free(pkey);
}

static int set_random_serialnumber(const char *function_name, X509 *x509) {
    // 作業領域を確保
    ASN1_INTEGER *serial_int = ASN1_INTEGER_new();
    if (serial_int == NULL) {
        return 0;
    }
    BIGNUM *serial_bn = BN_new();
    if (serial_bn == NULL) {
        ASN1_INTEGER_free(serial_int);
        return 0;
    }
    // ランダム値をシリアルナンバーとして設定
    int serial_rand_bits = 64;
    BN_pseudo_rand(serial_bn, serial_rand_bits, 0, 0);
    BN_to_ASN1_INTEGER(serial_bn, serial_int);
    int ret = X509_set_serialNumber(x509, serial_int);
    if (ret == 0) {
        sprintf(openssl_message, "%s: X509_set_serialNumber failed", function_name);
    }
    // リソースを解放
    ASN1_INTEGER_free(serial_int);
    BN_free(serial_bn);
    return ret;
}

X509 *prepare_x509_cert_new(const char *function_name, const char *available_days) {
    // 証明書格納領域を初期化
    X509 *x509_cert = X509_new();
    if (x509_cert == NULL) {
        sprintf(openssl_message, "%s: X509_new failed", function_name);
        return NULL;
    }
    // バージョンを設定（V3証明書）
    int n_version = 2;
    if (X509_set_version(x509_cert, n_version) == 0) {
        X509_free(x509_cert);
        sprintf(openssl_message, "%s: X509_set_version failed", function_name);
        return NULL;
    }
    // シリアルナンバーを設定（ランダム値）
    if (set_random_serialnumber(function_name, x509_cert) == 0) {
        X509_free(x509_cert);
        return NULL;
    }
    // 有効期限を設定
    int days = atoi(available_days);
    X509_gmtime_adj(X509_get_notBefore(x509_cert), 0);
    X509_gmtime_adj(X509_get_notAfter(x509_cert), 60 * 60 * 24 * days);

    return x509_cert;
}

X509_REQ *read_x509_req(const char *function_name, const char *csr_file_path) {
    // 証明書要求ファイルを開く
    FILE *csrfp = fopen(csr_file_path, "r");
    if (csrfp == NULL) {
        sprintf(openssl_message, "%s: fopen failed: %s", function_name, csr_file_path);
        return NULL;
    }
    // 証明書要求ファイルの内容を取得
    X509_REQ *x509_req = PEM_read_X509_REQ(csrfp, NULL, NULL, NULL);
    if (x509_req == NULL) {
        sprintf(openssl_message, "%s: PEM_read_X509_REQ failed", function_name);
    }
    // ファイルを閉じる
    fclose(csrfp);
    return x509_req;
}

int x509_req_copy_from_x509_req(const char *function_name, X509 *x509_cert, X509_REQ *x509_req) {
    // 証明書要求からSubjectを取得
    X509_NAME *subject = X509_REQ_get_subject_name(x509_req);
    if (subject == NULL) {
        sprintf(openssl_message, "%s: X509_REQ_get_subject_name failed", function_name);
        return 0;
    }
    // 証明書にIssuerとして設定
    if (X509_set_issuer_name(x509_cert, subject) == 0) {
        sprintf(openssl_message, "%s: X509_set_issuer_name failed", function_name);
        return 0;
    }
    // 証明書にIssuerとして設定
    if (X509_set_subject_name(x509_cert, subject) == 0) {
        sprintf(openssl_message, "%s: X509_set_subject_name failed", function_name);
        return 0;
    }
    return 1;
}

bool x509_cert_add_v3_extension(const char *function_name, X509 *x509_cert) {
    // 証明書に拡張属性を割当
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, x509_cert, x509_cert, NULL, NULL, 0);
    // 追加する拡張属性を生成
    X509_EXTENSION *ex = X509V3_EXT_conf_nid(NULL, &ctx,
                                             get_u2f_trans_ext_ble_oid(),
                                             U2F_TRANS_EXT_BLE_VALUE);
    if (ex == NULL) {
        sprintf(openssl_message, "%s: X509V3_EXT_conf_nid failed", function_name);
        return false;
    }
    // 証明書に拡張属性を追加
    int ret = X509_add_ext(x509_cert, ex, -1);
    if (ret == 0) {
        sprintf(openssl_message, "%s: X509_add_ext failed", function_name);
    }
    X509_EXTENSION_free(ex);
    return true;
}

bool x509_cert_setpkey_and_sign(const char *function_name, X509 *x509_cert, EVP_PKEY *pkey) {
    // 証明書に公開鍵を設定
    if (X509_set_pubkey(x509_cert, pkey) == 0) {
        sprintf(openssl_message, "%s: X509_set_pubkey failed", function_name);
        return false;
    }
    // 証明書に署名
    if (X509_sign(x509_cert, pkey, EVP_sha1()) < 1) {
        sprintf(openssl_message, "%s: x509_cert failed", function_name);
        return false;
    }
    return true;
}

int x509_cert_write(const char *function_name, X509 *x509_cert, const char *output_file_path) {
    // 証明書ファイルを開く
    FILE *outfp = fopen(output_file_path, "wb");
    if (outfp == NULL) {
        sprintf(openssl_message, "%s: fopen failed: %s", function_name, output_file_path);
        return 0;
    }
    // 証明書をファイルに書き出し
    int ret = i2d_X509_fp(outfp, x509_cert);
    if (ret == 0) {
        sprintf(openssl_message, "%s: i2d_X509_fp failed", function_name);
    }
    // ファイルを閉じる
    fclose(outfp);
    return ret;
}

bool create_selfcrt_crt_file(const char *output_file_path, const char *csr_file_path,
                             const char *privkey_file_path, const char *available_days) {
    const char *function_name = "create_selfcrt_crt_file";
    // 証明書格納領域を初期化（V3証明書）
    X509 *x509_cert = prepare_x509_cert_new(function_name, available_days);
    if (x509_cert == NULL) {
        free_resources_for_crt(x509_cert, NULL, NULL);
        return false;
    }
    // 証明書要求ファイルの内容を取得
    X509_REQ *x509_req = read_x509_req(function_name, csr_file_path);
    if (x509_req == NULL) {
        free_resources_for_crt(x509_cert, x509_req, NULL);
        return false;
    }
    // 証明書要求ファイルの内容を転記
    if (x509_req_copy_from_x509_req(function_name, x509_cert, x509_req) == false) {
        free_resources_for_crt(x509_cert, x509_req, NULL);
        return false;
    }
    // 拡張属性を設定
    if (x509_cert_add_v3_extension(function_name, x509_cert) == false) {
        free_resources_for_crt(x509_cert, x509_req, NULL);
        return false;
    }
    // EC鍵ファイルの内容を取得
    EVP_PKEY *pkey = read_private_key(function_name, privkey_file_path);
    if (pkey == NULL) {
        free_resources_for_crt(x509_cert, x509_req, pkey);
        return false;
    }
    // 証明書に公開鍵を設定＆署名
    if (x509_cert_setpkey_and_sign(function_name, x509_cert, pkey) == false) {
        free_resources_for_crt(x509_cert, x509_req, pkey);
        return false;
    }
    // 証明書をファイルに書き出し
    if (x509_cert_write(function_name, x509_cert, output_file_path) == 0) {
        free_resources_for_crt(x509_cert, x509_req, pkey);
        return false;
    }
    // リソースを解放
    sprintf(openssl_message, "%s: Self-signed certificate file created successfully.", function_name);
    free_resources_for_crt(x509_cert, x509_req, pkey);
    return true;
}
