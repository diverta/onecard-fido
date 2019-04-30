//
//  CBOREncoder.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#include "CBOREncoder.h"
#include "ECDH.h"
#include "FIDODefines.h"
#include "fido_blob.h"
#include "fido_crypto.h"
#include "debug_log.h"

// Values for COSE_Key format
//  Key type
#define COSE_KEY_LABEL_KTY  1
#define COSE_KEY_KTY_EC2    2
//  Curve type
#define COSE_KEY_LABEL_CRV  -1
#define COSE_KEY_CRV_P256   1
//  Key coordinate
#define COSE_KEY_LABEL_X    -2
#define COSE_KEY_LABEL_Y    -3
//  Signature algorithm
#define COSE_KEY_LABEL_ALG  3
#define COSE_ALG_ES256      -7

// ヘルスチェック実行用のテストデータ
static const char *challenge = "This is challenge";
static const char *rpid = "diverta.co.jp";
static const char *rpname = "Diverta inc.";
static const char *userid = "1234567890123456";
static const char *username = "username";
static const char *displayName = "User Name";
static const char *public_key_type = "public-key";

static uint8_t requestBytes[1024];
static size_t  requestBytesLength;

uint8_t *ctap2_cbor_encode_request_bytes(void) {
    return requestBytes;
}

size_t ctap2_cbor_encode_request_bytes_size(void) {
    return requestBytesLength;
}

uint8_t ctap2_cbor_encode_get_agreement_key(void) {
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_CLIENT_PIN;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    size_t map_elements_num = 2;
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // pinProtocol(0x01): 0x01
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // subCommand(0x02): getKeyAgreement(0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_uint(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

static uint8_t encode_cose_pubkey(CborEncoder *encoder, uint8_t *x, uint8_t *y, int32_t alg) {
    CborError   ret;
    CborEncoder map;
    
    // Map初期化
    ret = cbor_encoder_create_map(encoder, &map, 5);
    if (ret != CborNoError) {
        return ret;
    }
    // Key type
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_KTY);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, COSE_KEY_KTY_EC2);
    if (ret != CborNoError) {
        return ret;
    }
    // Signature algorithm
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_ALG);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, alg);
    if (ret != CborNoError) {
        return ret;
    }
    // Curve type
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_CRV);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, COSE_KEY_CRV_P256);
    if (ret != CborNoError) {
        return ret;
    }
    // x-coordinate
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_X);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_byte_string(&map, x, 32);
    if (ret != CborNoError) {
        return ret;
    }
    // y-coordinate
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_Y);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_byte_string(&map, y, 32);
    if (ret != CborNoError) {
        return ret;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return ret;
    }
    
    return CborNoError;
}

static uint8_t add_encoded_cosekey_to_map(CborEncoder *encoder) {
    // CBORエンコード実行
    uint8_t *x = ECDH_public_key_X();
    uint8_t *y = ECDH_public_key_Y();
    int32_t alg = COSE_ALG_ES256;
    uint8_t ret = encode_cose_pubkey(encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t generate_set_pin_cbor(bool change_pin) {
    // Mapに格納する要素数
    size_t map_elements_num;
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_CLIENT_PIN;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    if (change_pin) {
        map_elements_num = 6;
    } else {
        map_elements_num = 5;
    }
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinProtocol(0x01): 0x01
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // subCommand(0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    if (change_pin) {
        ret = cbor_encode_uint(&map, CTAP2_SUBCMD_CLIENT_PIN_CHANGE);
    } else {
        ret = cbor_encode_uint(&map, CTAP2_SUBCMD_CLIENT_PIN_SET);
    }
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // keyAgreement(0x03)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = add_encoded_cosekey_to_map(&map);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    // pinAuth(0x04) 16 bytes
    ret = cbor_encode_int(&map, 0x04);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, pin_auth(), 16);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // newPinEnc(0x05)
    ret = cbor_encode_int(&map, 0x05);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, new_pin_enc(), new_pin_enc_size());
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinHashEnc(0x06) 16 bytes
    if (change_pin) {
        ret = cbor_encode_int(&map, 0x06);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_byte_string(&map, pin_hash_enc(), 16);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_client_pin_set_or_change(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, char *new_pin, char *old_pin) {
    // ECDHキーペアを新規作成し、受領した公開鍵から共通鍵を生成
    if (ECDH_create_shared_secret_key(agreement_pubkey_X, agreement_pubkey_Y) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinHashEncを生成
    bool change_pin = (old_pin != NULL);
    if (change_pin) {
        if (generate_pin_hash_enc(old_pin) != CTAP1_ERR_SUCCESS) {
            return CTAP1_ERR_OTHER;
        }
    }
    // newPinEncを生成
    if (generate_new_pin_enc(new_pin) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuthを生成
    if (generate_pin_auth(change_pin) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // リクエストCBORを生成
    return generate_set_pin_cbor(change_pin);
}

static uint8_t generate_get_pin_token_cbor(void) {
    // Mapに格納する要素数
    size_t map_elements_num;
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_CLIENT_PIN;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    map_elements_num = 4;
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinProtocol(0x01): 0x01
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // subCommand(0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // keyAgreement(0x03)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = add_encoded_cosekey_to_map(&map);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    // pinHashEnc(0x06) 16 bytes
    ret = cbor_encode_int(&map, 0x06);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, pin_hash_enc(), 16);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_client_pin_token_get(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, char *cur_pin) {
    // ECDHキーペアを新規作成し、受領した公開鍵から共通鍵を生成
    if (ECDH_create_shared_secret_key(agreement_pubkey_X, agreement_pubkey_Y) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinHashEncを生成
    if (generate_pin_hash_enc(cur_pin) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // リクエストCBORを生成
    return generate_get_pin_token_cbor();
}

static uint8_t encode_rp(CborEncoder *encoder) {
    // Mapに格納する要素数 = 2
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(encoder, &map, 2);
    if (ret == CborNoError) {
        // id
        ret = cbor_encode_text_stringz(&map, "id");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, rpid);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // name
        ret = cbor_encode_text_stringz(&map, "name");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, rpname);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t encode_user(CborEncoder *encoder) {
    // Mapに格納する要素数 = 3
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(encoder, &map, 3);
    if (ret == CborNoError) {
        // id
        ret = cbor_encode_text_stringz(&map, "id");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_byte_string(&map, (uint8_t *)userid, strlen(userid));
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // name
        ret = cbor_encode_text_stringz(&map, "name");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, username);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // displayName
        ret = cbor_encode_text_stringz(&map, "displayName");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, displayName);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t encode_options(CborEncoder *encoder, bool up) {
    // Mapに格納する要素数 = 3
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(encoder, &map, 3);
    if (ret == CborNoError) {
        // rk
        ret = cbor_encode_text_stringz(&map, "rk");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_boolean(&map, false);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // uv
        ret = cbor_encode_text_stringz(&map, "uv");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_boolean(&map, false);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // up
        ret = cbor_encode_text_stringz(&map, "up");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_boolean(&map, up);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t encode_pubkey_cred_params(CborEncoder *encoder) {
    // 配列要素数 = 1;
    CborEncoder cborarray;
    CborError ret = cbor_encoder_create_array(encoder, &cborarray, 1);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // Mapに格納する要素数 = 2
    CborEncoder map;
    ret = cbor_encoder_create_map(&cborarray, &map, 2);
    if (ret == CborNoError) {
        // alg
        ret = cbor_encode_text_stringz(&map, "alg");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_int(&map, COSE_ALG_ES256);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // type
        ret = cbor_encode_text_stringz(&map, "type");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, public_key_type);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    ret = cbor_encoder_close_container(&cborarray, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // 配列をクローズ
    ret = cbor_encoder_close_container(encoder, &cborarray);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t generate_make_credential_cbor(void) {
    // Mapに格納する要素数
    size_t map_elements_num;
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_MAKE_CREDENTIAL;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    map_elements_num = 7;
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // clientDataHash(0x01) 32 bytes
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, client_data_hash(), 32);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // rp(0x02) PublicKeyCredentialRpEntity
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_rp(&map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // user(0x03) PublicKeyCredentialRpEntity
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_user(&map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pubKeyCredParams(0x04) CBOR Array
    ret = cbor_encode_int(&map, 0x04);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_pubkey_cred_params(&map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // options(0x07) Map of authenticator options
    ret = cbor_encode_int(&map, 0x07);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_options(&map, false);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuth(0x08) 16bytes
    ret = cbor_encode_int(&map, 0x08);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, pin_auth(), 16);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinProtocol(0x09): 0x01
    ret = cbor_encode_int(&map, 0x09);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_make_credential(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, uint8_t *pin_token) {
    // clientDataHashを生成
    if (generate_client_data_hash(challenge) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuthを生成
    if (generate_pin_auth_from_client_data(pin_token, client_data_hash()) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // リクエストCBORを生成
    return generate_make_credential_cbor();
}

static uint8_t encode_allow_list(
    CborEncoder *encoder, uint8_t *credential_id, size_t credential_id_size) {
    // 配列要素数 = 1;
    CborEncoder cborarray;
    CborError ret = cbor_encoder_create_array(encoder, &cborarray, 1);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // Mapに格納する要素数 = 2
    CborEncoder map;
    ret = cbor_encoder_create_map(&cborarray, &map, 2);
    if (ret == CborNoError) {
        // type
        ret = cbor_encode_text_stringz(&map, "type");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_text_stringz(&map, public_key_type);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // id
        ret = cbor_encode_text_stringz(&map, "id");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_byte_string(&map, credential_id, credential_id_size);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    ret = cbor_encoder_close_container(&cborarray, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // 配列をクローズ
    ret = cbor_encoder_close_container(encoder, &cborarray);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t generate_get_assertion_cbor(uint8_t *credential_id, size_t credential_id_size) {
    // Mapに格納する要素数
    size_t map_elements_num;
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_GET_ASSERTION;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    map_elements_num = 6;
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // rpId(0x01) String
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_text_stringz(&map, rpid);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // clientDataHash(0x02) 32 bytes
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, client_data_hash(), 32);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // allowList(0x03) Sequence of PublicKeyCredentialDescriptors
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_allow_list(&map, credential_id, credential_id_size);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // options(0x05) Map of authenticator options
    ret = cbor_encode_int(&map, 0x05);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = encode_options(&map, true);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuth(0x06) 16bytes
    ret = cbor_encode_int(&map, 0x06);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, pin_auth(), 16);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinProtocol(0x07): 0x01
    ret = cbor_encode_int(&map, 0x07);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_get_assertion(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, uint8_t *pin_token,
    uint8_t *credential_id, size_t credential_id_size) {
    // clientDataHashを生成
    if (generate_client_data_hash(challenge) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuthを生成
    if (generate_pin_auth_from_client_data(pin_token, client_data_hash()) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // リクエストCBORを生成
    return generate_get_assertion_cbor(credential_id, credential_id_size);
}
