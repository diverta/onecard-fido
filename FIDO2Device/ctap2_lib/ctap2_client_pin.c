/* 
 * File:   ctap2_client_pin.c
 * Author: makmorit
 *
 * Created on 2019/02/18, 11:05
 */
#include "ctap2_cbor.h"
#include "ctap2_define.h"
#include "fido_command.h"
#include "fido_command_common.h"
#include "fido_common.h"
#include "fido_ctap2_command.h"
#include "fido_define.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_cbor_encode.h"
#include "ctap2_pubkey_credential.h"
#include "ctap2_client_pin_token.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ctap2_client_pin);
#endif

// for debug cbor data
#define LOG_HEXDUMP_DEBUG_CBOR      false
#define LOG_DEBUG_CBOR_REQUEST      false
#define LOG_DEBUG_CALCULATED_HMAC   false
#define LOG_DEBUG_PIN_CODE          false

// デコードされた
// authenticatorClientPIN
// リクエストデータを保持する構造体
#define NEW_PIN_ENC_MAX_SIZE    256
#define NEW_PIN_ENC_MIN_SIZE    64
#define PIN_HASH_ENC_SIZE       16
static struct {
    uint8_t pinProtocol;
    uint8_t subCommand;
    CTAP_COSE_KEY cose_key;
    uint8_t pinAuth[PIN_AUTH_SIZE];
    size_t  pinAuthSize;
    uint8_t newPinEnc[NEW_PIN_ENC_MAX_SIZE];
    size_t  newPinEncSize;
    uint8_t pinHashEnc[PIN_HASH_ENC_SIZE];
    size_t  pinHashEncSize;
} ctap2_request;

// レスポンス長を保持
static size_t m_response_length;

// サブコマンド定義
#define subcmd_GetRetries       0x01
#define subcmd_GetKeyAgreement  0x02
#define subcmd_SetPin           0x03
#define subcmd_ChangePin        0x04
#define subcmd_GetPinToken      0x05

// 復号化されたPINコードを保持
static uint8_t pin_code[NEW_PIN_ENC_MAX_SIZE];
static size_t  pin_code_size;

// PINコードの長さ
#define NEW_PIN_MAX_SIZE        64
#define NEW_PIN_MIN_SIZE        4

// PINコードハッシュを保持
static uint8_t pin_code_hash[SHA_256_HASH_SIZE];
static size_t  pin_code_hash_size;
static uint8_t hmac[HMAC_SHA_256_SIZE];

// PINミスマッチ最大連続回数
#define PIN_MISMATCH_COUNT_MAX 3

// PINミスマッチが連続した回数を保持
static uint8_t pin_mismatch_count = 0;

// changePINコマンドのステータスコードを保持
//   コマンドの処理成功／失敗にかかわらず、
//   PINリトライカウンターレコードの更新をはさんでレスポンスするため、
//   外部変数にステータスコードを保持させる必要あり
uint8_t check_pin_status_code;

static void debug_decoded_request()
{
#if LOG_DEBUG_CBOR_REQUEST
    fido_log_debug("pinProtocol(0x%02x) subCommand(0x%02x)", ctap2_request.pinProtocol, ctap2_request.subCommand);

    fido_log_debug("keyAgreement: alg(%d) curve(%d) public key(64 bytes):",
        ctap2_request.cose_key.alg, ctap2_request.cose_key.crv);
    fido_log_print_hexdump_debug(ctap2_request.cose_key.key.x, 32);
    fido_log_print_hexdump_debug(ctap2_request.cose_key.key.y, 32);

    fido_log_debug("pinAuth(%dbytes):", PIN_AUTH_SIZE);
    fido_log_print_hexdump_debug(ctap2_request.pinAuth, PIN_AUTH_SIZE);

    fido_log_debug("newPinEnc(%dbytes):", ctap2_request.newPinEncSize);
    int j, k;
    int max = (ctap2_request.newPinEncSize < NEW_PIN_ENC_MAX_SIZE) ? ctap2_request.newPinEncSize : NEW_PIN_ENC_MAX_SIZE;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        fido_log_print_hexdump_debug(ctap2_request.newPinEnc + j, (k < 64) ? k : 64);
    }

    fido_log_debug("pinHashEnc(%dbytes):", PIN_HASH_ENC_SIZE);
    fido_log_print_hexdump_debug(ctap2_request.pinHashEnc, PIN_HASH_ENC_SIZE);
#endif
}

uint8_t ctap2_client_pin_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     err;
    uint8_t     i;
    int         key;
    size_t      sz;

#if LOG_HEXDUMP_DEBUG_CBOR
    fido_log_debug("authenticatorClientPIN request cbor(%d bytes):", cbor_data_length);
    int j, k;
    int max = (cbor_data_length < 288) ? cbor_data_length : 288;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        fido_log_print_hexdump_debug(cbor_data_buffer + j, (k < 64) ? k : 64);
    }
#else
    (void)cbor_data_buffer;
    (void)cbor_data_length;
#endif
    // 必須項目チェック済みフラグを初期化
    uint8_t must_item_flag = 0;

    // リクエスト格納領域初期化
    memset(&ctap2_request, 0x00, sizeof(ctap2_request));

    // CBOR parser初期化
    //   0x0fff: CborValidateCanonicalFormat
    ret = ctap2_cbor_parser_init(cbor_data_buffer, cbor_data_length, 0x0fff, &parser, &it);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    type = cbor_value_get_type(&it);
    if (type != CborMapType) {
        return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
    }

    ret = cbor_value_enter_container(&it, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(&it, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        type = cbor_value_get_type(&map);
        if (type != CborIntegerType) {
            return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
        }
        ret = cbor_value_get_int_checked(&map, &key);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        switch(key) {
            case 1:
                // pinProtocol (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_get_int_checked(&map, (int *)&ctap2_request.pinProtocol) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x01;
                break;
            case 2:
                // subCommand (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_get_int_checked(&map, (int *)&ctap2_request.subCommand) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x02;
                break;
            case 3:
                // keyAgreement (COSE_Key)
                err = parse_cose_pubkey(&map, &ctap2_request.cose_key);
                if (err != CTAP1_ERR_SUCCESS) {
                    return err;
                }
                break;
            case 4:
                // pinAuth (Byte Array)
                err = parse_fixed_byte_string(&map, ctap2_request.pinAuth, PIN_AUTH_SIZE);
                if (err != CTAP1_ERR_SUCCESS) {
                    return err;
                }
                ctap2_request.pinAuthSize = PIN_AUTH_SIZE;
                break;
            case 5:
                // newPinEnc (Byte Array)
                if (cbor_value_get_type(&map) != CborByteStringType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_calculate_string_length(&map, &sz) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                if (sz > NEW_PIN_ENC_MAX_SIZE || sz < NEW_PIN_ENC_MIN_SIZE) {
                    return CTAP2_ERR_PIN_POLICY_VIOLATION;
                }
                ctap2_request.newPinEncSize = sz;
                sz = NEW_PIN_ENC_MAX_SIZE;
                if (cbor_value_copy_byte_string(&map, ctap2_request.newPinEnc, &sz, NULL) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                break;
            case 6:
                // pinHashEnc (Byte Array)
                err = parse_fixed_byte_string(&map, ctap2_request.pinHashEnc, PIN_HASH_ENC_SIZE);
                if (err != CTAP1_ERR_SUCCESS) {
                    return err;
                }
                ctap2_request.pinHashEncSize = PIN_HASH_ENC_SIZE;
                break;
            default:
                break;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

    debug_decoded_request();

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x03) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

void perform_get_retry_counter(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    uint32_t retry_counter = 8;

    // PINコードハッシュ、リトライカウンターをFlash ROMから取得
    if (fido_flash_client_pin_store_hash_read()) {
        retry_counter = fido_flash_client_pin_store_retry_counter();
    }
    
    // レスポンスをエンコード
    uint8_t ctap2_status = ctap2_cbor_encode_response_retry_counter(encoded_buff, encoded_buff_size, retry_counter);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをCTAP2クライアントに戻す
    // （レスポンス長はステータス1バイト＋CBORレスポンス長とする）
    m_response_length = *encoded_buff_size + 1;
    fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, m_response_length);
    fido_log_debug("getRetries: retry counter=%d", retry_counter);
}

void perform_get_key_agreement(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 鍵交換用キーペアが未生成の場合は新規生成
    // (再生成は要求しない)
    fido_command_sskey_init(false);

    // レスポンスをエンコード
    uint8_t ctap2_status = ctap2_cbor_encode_response_key_agreement(encoded_buff, encoded_buff_size);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをCTAP2クライアントに戻す
    // （レスポンス長はステータス1バイト＋CBORレスポンス長とする）
    m_response_length = *encoded_buff_size + 1;
    fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, m_response_length);
    fido_log_debug("getKeyAgreement: public key generate success");
}

uint8_t verify_pin_auth(void)
{
    // 共通鍵ハッシュを利用し、
    // CTAP2クライアントから受領したPINデータを
    // HMAC SHA-256アルゴリズムでハッシュ化
    fido_command_sskey_calculate_hmac_sha256(
        ctap2_request.newPinEnc, ctap2_request.newPinEncSize,
        ctap2_request.pinHashEnc, ctap2_request.pinHashEncSize, 
        hmac);

#if LOG_DEBUG_CALCULATED_HMAC
    fido_log_debug("Calculated hmac(%dbytes):", ctap2_request.pinAuthSize);
    fido_log_print_hexdump_debug(hmac, ctap2_request.pinAuthSize);
    fido_log_debug("pinAuth(%dbytes):", ctap2_request.pinAuthSize);
    fido_log_print_hexdump_debug(ctap2_request.pinAuth, ctap2_request.pinAuthSize);
#endif

    // クライアントから受信したpinAuth（16バイト）を、
    // PINデータから生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(hmac, ctap2_request.pinAuth, ctap2_request.pinAuthSize) != 0) {
        return CTAP2_ERR_PIN_AUTH_INVALID;
    }

    fido_log_debug("pinAuth verification success");
    return CTAP1_ERR_SUCCESS;
}

uint8_t calculate_pin_code_hash(void)
{
    uint8_t *pin_buff      = pin_code;
    size_t   pin_buff_size = pin_code_size;

    // PINバッファには後ろが 0 埋めされているため、
    // その部分を長さにカウントしないようにする
    uint8_t pin_len = pin_buff_size - 1;
    while (pin_buff[pin_len] == 0 && 0 < pin_len) {
        pin_len--;
    }

    // PINコードの長さを検証
    pin_len += 1;
    if (pin_len < NEW_PIN_MIN_SIZE || pin_len >= NEW_PIN_MAX_SIZE) {
        return CTAP2_ERR_PIN_POLICY_VIOLATION;
    }

    // 後ろの 0 埋めを考慮しないPINコード長を設定
    pin_code_size = pin_len;

    // PINコードをSHA-256ハッシュ化し、
    // PINコードハッシュ（32バイト）を作成
    pin_code_hash_size = SHA_256_HASH_SIZE;
    fido_command_calc_hash_sha256(pin_code, pin_code_size, pin_code_hash, &pin_code_hash_size);

#if LOG_DEBUG_PIN_CODE
    fido_log_debug("PIN code(%dbytes):", pin_len);
    fido_log_print_hexdump_debug(pin_code, pin_len);
    fido_log_debug("PIN code hash(%dbytes):", pin_code_hash_size);
    fido_log_print_hexdump_debug(pin_code_hash, pin_code_hash_size);
#endif

    return CTAP1_ERR_SUCCESS;
}

static bool check_pin_code_hash(void)
{
    // PINコードハッシュ、リトライカウンターをFlash ROMから取得
    if (fido_flash_client_pin_store_hash_read() == false) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP2_ERR_PIN_NOT_SET, 1);
        return false;
    }

    // リトライカウンターを１減らす
    uint32_t retry_counter = fido_flash_client_pin_store_retry_counter();
    if (retry_counter > 0) {
        retry_counter--;
    }
    fido_log_debug("retry counter remains %d times", retry_counter);

    // CTAP2クライアントから受け取った旧いPINコードを、
    // 共通鍵ハッシュを使用して復号化
    pin_code_size = fido_command_sskey_aes_256_cbc_decrypt(
        ctap2_request.pinHashEnc, ctap2_request.pinHashEncSize, pin_code);
    if (pin_code_size != ctap2_request.pinHashEncSize) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return false;
    }

    // 復号化された旧いPINコードと、
    // Flash ROMに保管されているPINコードを比較し、
    // 一致していれば後続の処理を行う
    if (memcmp(pin_code, fido_flash_client_pin_store_pin_code_hash(), pin_code_size) == 0) {
        // PINミスマッチ連続回数をゼロクリア
        pin_mismatch_count = 0;
        fido_log_debug("PIN code hash matching test OK");
        check_pin_status_code = CTAP1_ERR_SUCCESS;
        return true;
    }

    // 一致しない場合はキーペアを再生成
    fido_command_sskey_init(true);

    // エラーレスポンスを待避
    if (retry_counter == 0) {
        // リトライカウンターが0の場合
        fido_log_error("PIN code hash matching NG (max retry count reached)");
        check_pin_status_code = CTAP2_ERR_PIN_BLOCKED;

    } else if (++pin_mismatch_count == PIN_MISMATCH_COUNT_MAX) {
        // PINミスマッチが連続した回数をカウントアップし、
        // ３回に達した場合
        fido_log_error("PIN code hash matching NG (consecutive 3 times)");
        check_pin_status_code = CTAP2_ERR_PIN_AUTH_BLOCKED;
        // アプリケーション全体をロックし、
        // システムリセットが必要である旨をユーザーに知らせる
        fido_command_abort_flag_set(true);

    } else {
        fido_log_error("PIN code hash matching NG");
        check_pin_status_code = CTAP2_ERR_PIN_INVALID;
    }

    // リトライカウンターをFlash ROMに更新登録
    if (fido_flash_client_pin_store_hash_write(NULL, retry_counter) == false) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return false;
    }

    // 後続の処理は行わない
    return false;
}

void perform_set_pin(uint8_t *encoded_buff, size_t *encoded_buff_size, bool pin_change)
{
    // CTAP2クライアントから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    uint8_t ctap2_status = fido_command_sskey_generate((uint8_t *)&ctap2_request.cose_key.key);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 鍵交換用キーペアが未生成の場合は
        // エラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // CTAP2クライアントから受け取ったHMACハッシュを、
    // 共通鍵ハッシュを使用して検証
    ctap2_status = verify_pin_auth();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    if (pin_change) {
        // PIN変更リクエストの場合は、PINのマッチングチェックを実行
        // チェックがNGの場合は、
        // リトライカウンターを１減らしてFlash ROMに反映
        if (check_pin_code_hash() == false) {
            return;
        }
    }

    // CTAP2クライアントから受け取ったPINコードを、
    // 共通鍵ハッシュを使用して復号化
    pin_code_size = fido_command_sskey_aes_256_cbc_decrypt(
        ctap2_request.newPinEnc, ctap2_request.newPinEncSize, pin_code);
    if (pin_code_size != ctap2_request.newPinEncSize) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return;
    }

    // PINコードの長さをチェックし、
    // OKであればPINコードハッシュを生成
    ctap2_status = calculate_pin_code_hash();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをエンコード
    ctap2_status = ctap2_cbor_encode_response_set_pin(encoded_buff, encoded_buff_size);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをモジュール変数に設定しておく
    // （レスポンス長はステータス1バイト＋CBORレスポンス長とする）
    m_response_length = *encoded_buff_size + 1;

    // PINリトライカウンターをFlash ROMに保管
    // リトライカウンターの初期値は８とする
    uint32_t retry_counter = 8;
    if (fido_flash_client_pin_store_hash_write(pin_code_hash, retry_counter) == false) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return;
    }
}


void perform_get_pin_token(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // リトライカウンターをFlash ROMから取得
    if (fido_flash_client_pin_store_hash_read() == false) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP2_ERR_PIN_NOT_SET, 1);
        return;
    }

    uint32_t retry_counter = fido_flash_client_pin_store_retry_counter();
    if (retry_counter == 0) {
        // リトライカウンターが0の場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP2_ERR_PIN_BLOCKED, 1);
        return;
    }

    // CTAP2クライアントから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    uint8_t ctap2_status = fido_command_sskey_generate((uint8_t *)&ctap2_request.cose_key.key);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 鍵交換用キーペアが未生成の場合は
        // エラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // PINのマッチングチェックを実行
    // チェックがNGの場合は、
    // リトライカウンターを１減らしてFlash ROMに反映
    if (check_pin_code_hash() == false) {
        // レスポンス長は1バイトとする（ステータスコードのみ）
        m_response_length = 1;
        return;
    }

    // pinTokenを共通鍵で暗号化
    ctap2_status = ctap2_client_pin_token_encode();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをエンコード
    ctap2_status = ctap2_cbor_encode_response_get_pin_token(encoded_buff, encoded_buff_size);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return;
    }

    // レスポンスをモジュール変数に設定しておく
    // （レスポンス長はステータス1バイト＋CBORレスポンス長とする）
    m_response_length = *encoded_buff_size + 1;

    // PINリトライカウンターをFlash ROMに保管
    // リトライカウンターの初期値は８とする
    retry_counter = 8;
    if (fido_flash_client_pin_store_hash_write(NULL, retry_counter) == false) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return;
    }
}

void ctap2_client_pin_perform_subcommand(uint8_t *response_buffer, size_t response_buffer_size)
{
    fido_log_debug("authenticatorClientPIN start: subcommand(0x%02x)", ctap2_request.subCommand);

    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = response_buffer_size - 1;

    switch (ctap2_request.subCommand) {
        case subcmd_GetRetries:
            perform_get_retry_counter(cbor_data_buffer, &cbor_data_length);
            break;
        case subcmd_GetKeyAgreement:
            perform_get_key_agreement(cbor_data_buffer, &cbor_data_length);
            break;
        case subcmd_SetPin:
            perform_set_pin(cbor_data_buffer, &cbor_data_length, false);
            break;
        case subcmd_ChangePin:
            perform_set_pin(cbor_data_buffer, &cbor_data_length, true);
            break;
        case subcmd_GetPinToken:
            perform_get_pin_token(cbor_data_buffer, &cbor_data_length);
            break;
        default:
            // サポートされていないサブコマンドなので
            // エラーステータスを戻す
            fido_ctap2_command_send_response(CTAP1_ERR_INVALID_COMMAND, 1);
            break;
    }
}

void ctap2_client_pin_send_response(void)
{
    switch (ctap2_request.subCommand) {
        case subcmd_SetPin:
            // レスポンスを戻す
            fido_log_debug("setPIN: PIN hash store success");
            fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, m_response_length);
            // PINが新規設定されたので、
            // PINミスマッチ連続回数をゼロクリア
            pin_mismatch_count = 0;
            break;
        case subcmd_ChangePin:
            // レスポンスを戻す
            fido_log_debug("changePIN: PIN hash store success");
            fido_ctap2_command_send_response(check_pin_status_code, m_response_length);
            break;
        case subcmd_GetPinToken:
            // レスポンスを戻す
            fido_log_debug("getPINToken: retry counter store success");
            fido_ctap2_command_send_response(check_pin_status_code, m_response_length);
            break;
        default:
            break;
    }
}

void ctap2_client_pin_response_sent(void)
{
    switch (ctap2_request.subCommand) {
        case subcmd_GetRetries:
            fido_log_info("authenticatorClientPIN(getRetries) end");
            break;
        case subcmd_GetKeyAgreement:
            fido_log_info("authenticatorClientPIN(getKeyAgreement) end");
            break;
        case subcmd_SetPin:
            fido_log_info("authenticatorClientPIN(setPIN) end");
            break;
        case subcmd_ChangePin:
            fido_log_info("authenticatorClientPIN(changePIN) end");
            break;
        case subcmd_GetPinToken:
            fido_log_info("authenticatorClientPIN(getPINToken) end");
            break;
        default:
            break;
    }
}

void ctap2_client_pin_init(void)
{
    // PINトークンとキーペアを再生成
    ctap2_client_pin_token_init(true);
    fido_command_sskey_init(true);
}
