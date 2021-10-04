/* 
 * File:   ctap2_cbor_authgetinfo.c
 * Author: makmorit
 *
 * Created on 2018/12/24, 9:32
 */
#include "ctap2_cbor.h"
#include "fido_common.h"
#include "ctap2_common.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ctap2_cbor_authgetinfo);
#endif

#define NUM_OF_CBOR_ELEMENTS        6
#define NUM_OF_VERSIONS             2
#define NUM_OF_OPTIONS              4

#define RESP_versions               0x1
#define RESP_aaguid                 0x3
#define RESP_options                0x4
#define RESP_maxMsgSize             0x5
#define RESP_pinProtocols           0x6

//
// aaguid（仮の値です）
//   0x2b2ecbb4-59b4-44fa-868d-a072485d8ae0
// FIDOアライアンス提供のファイル
//   Virtual Secp256R1 FIDO2 Conformance Testing CTAP2 Authenticator with Self(surrogate) attestation.json
// の記述内容と整合させています。
//
static uint8_t CTAP_AAGUID[] = {
    0x2b, 0x2e, 0xcb, 0xb4, 0x59, 0xb4, 0x44, 0xfa, 0x86, 0x8d, 0xa0, 0x72, 0x48, 0x5d, 0x8a, 0xe0
};

uint8_t *ctap2_cbor_authgetinfo_aaguid(void)
{
    return CTAP_AAGUID;
}

size_t ctap2_cbor_authgetinfo_aaguid_size(void)
{
    return sizeof(CTAP_AAGUID);
}

static bool encode_authgetinfo_response_message(CborEncoder *encoder)
{
    CborError   ret;
    CborEncoder array;
    CborEncoder map;
    CborEncoder options;
    CborEncoder pins;

    ret = cbor_encoder_create_map(encoder, &map, NUM_OF_CBOR_ELEMENTS);
    if (ret == CborNoError) {
        // versions
        ret = cbor_encode_uint(&map, RESP_versions);
        if (ret == CborNoError) {
            ret = cbor_encoder_create_array(&map, &array, NUM_OF_VERSIONS);
            if (ret == CborNoError) {
                ret = cbor_encode_text_stringz(&array, U2F_V2_VERSION_STRING);
                if (ret != CborNoError) {
                    return false;
                }

                ret = cbor_encode_text_stringz(&array, FIDO_2_0_VERSION_STRING);
                if (ret != CborNoError) {
                    return false;
                }
            }

            ret = cbor_encoder_close_container(&map, &array);
            if (ret != CborNoError) {
                return false;
            }
        }

        // extensions (0x02)
        ret = cbor_encode_uint(&map, 0x02);
        if (ret == CborNoError) {
            ret = cbor_encoder_create_array(&map, &array, 1);
            if (ret == CborNoError) {
                ret = cbor_encode_text_stringz(&array, "hmac-secret");
                if (ret != CborNoError) {
                    return false;
                }
            }

            ret = cbor_encoder_close_container(&map, &array);
            if (ret != CborNoError) {
                return false;
            }
        }

        // aaguid
        ret = cbor_encode_uint(&map, RESP_aaguid);
        if (ret == CborNoError) {
            ret = cbor_encode_byte_string(&map, CTAP_AAGUID, sizeof(CTAP_AAGUID));
            if (ret != CborNoError) {
                return false;
            }
        }

        // options
        ret = cbor_encode_uint(&map, RESP_options);
        if (ret == CborNoError) {
            ret = cbor_encoder_create_map(&map, &options, NUM_OF_OPTIONS);
            if (ret == CborNoError) {
                ret = cbor_encode_text_stringz(&options, "plat");
                if (ret == CborNoError) {
                    // Not attached to platform
                    ret = cbor_encode_boolean(&options, false);
                    if (ret != CborNoError) {
                        return false;
                    }
                }

                ret = cbor_encode_text_stringz(&options, "rk");
                if (ret == CborNoError) {
                    // State-less device, requires allowList parameter.
                    ret = cbor_encode_boolean(&options, false);
                    if (ret != CborNoError) {
                        return false;
                    }
                }

                ret = cbor_encode_text_stringz(&options, "up");
                if (ret == CborNoError) {
                    // Capable of testing user presence
                    ret = cbor_encode_boolean(&options, true);
                    if (ret != CborNoError) {
                        return false;
                    }
                }

                /*
                 * 生体認証機能を装備していない認証器は、
                 * パラメーター 'uv' をレスポンスする必要無し
                 * See: https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html#authenticatorGetInfo
                 * 
                ret = cbor_encode_text_stringz(&options, "uv");
                if (ret == CborNoError) {
                    ret = cbor_encode_boolean(&options, false);
                    if (ret != CborNoError) {
                        return false;
                    }
                }
                 */

                ret = cbor_encode_text_stringz(&options, "clientPin");
                if (ret == CborNoError) {
                    // 認証器にPINが設定されているかどうかチェックし、
                    // 設定の有無を 'clientPin' に設定
                    ret = cbor_encode_boolean(&options, fido_flash_client_pin_store_pin_code_exist());
                    if (ret != CborNoError) {
                        return false;
                    }
                }
            }

            ret = cbor_encoder_close_container(&map, &options);
            if (ret != CborNoError) {
                return false;
            }
        }

        // maxMsgSize
        ret = cbor_encode_uint(&map, RESP_maxMsgSize);
        if (ret == CborNoError) {
            ret = cbor_encode_int(&map, CTAP2_MAX_MESSAGE_SIZE);
            if (ret != CborNoError) {
                return false;
            }
        }

        // pinProtocols
        ret = cbor_encode_uint(&map, RESP_pinProtocols);
        if (ret == CborNoError) {
            ret = cbor_encoder_create_array(&map, &pins, 1);
            if (ret == CborNoError) {
                ret = cbor_encode_int(&pins, 1);
                if (ret != CborNoError) {
                    return false;
                }
            }

            ret = cbor_encoder_close_container(&map, &pins);
            if (ret != CborNoError) {
                return false;
            }
        }
    }

    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return false;
    }
    return true;
}

uint8_t ctap2_cbor_authgetinfo_encode_request(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 作業領域初期化
    memset(encoded_buff, 0x00, *encoded_buff_size);
    
    // CBORエンコーダー初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // CBORエンコード実行
    if (encode_authgetinfo_response_message(&encoder) == false) {
        // エラーコード（CTAP1_ERR_OTHER）を戻す
        fido_log_error("Encoding failed");
        return CTAP1_ERR_OTHER;
    }

    // エンコードされたバッファのサイズを設定
    *encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    fido_log_debug("Encoding success (%d bytes)", *encoded_buff_size);
    return CTAP1_ERR_SUCCESS;
}
