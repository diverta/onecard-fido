/* 
 * File:   ctap2_cbor_authgetinfo.c
 * Author: makmorit
 *
 * Created on 2018/12/24, 9:32
 */
#include "sdk_common.h"

#include <stdbool.h>
#include <stdint.h>
#include "cbor.h"
#include "fido_common.h"
#include "ctap2.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_cbor_authgetinfo
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define NUM_OF_CBOR_ELEMENTS        5
#define NUM_OF_VERSIONS             2
#define NUM_OF_OPTIONS              5

#define RESP_versions               0x1
#define RESP_aaguid                 0x3
#define RESP_options                0x4
#define RESP_maxMsgSize             0x5
#define RESP_pinProtocols           0x6

//
// aaguid（仮の値です）
//   0xf8a011f38c0a4d15800617111f9edc7d
//
static uint8_t CTAP_AAGUID[] = {
    0xf8, 0xa0, 0x11, 0xf3, 0x8c, 0x0a, 0x4d, 0x15, 0x80, 0x06, 0x17, 0x11, 0x1f, 0x9e, 0xdc, 0x7d
};

uint8_t *ctap2_cbor_authgetinfo_aaguid()
{
    return CTAP_AAGUID;
}

size_t ctap2_cbor_authgetinfo_aaguid_size()
{
    return sizeof(CTAP_AAGUID);
}

static bool encode_authgetinfo_response_message(CborEncoder *encoder)
{
    int ret;
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
                ret = cbor_encode_text_stringz(&array, "U2F_V2");
                if (ret != CborNoError) {
                    return false;
                }

                ret = cbor_encode_text_stringz(&array, "FIDO_2_0");
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

                ret = cbor_encode_text_stringz(&options, "uv");
                if (ret == CborNoError) {
                    // NOT [yet] capable of verifying user
                    ret = cbor_encode_boolean(&options, false);
                    if (ret != CborNoError) {
                        return false;
                    }
                }

                ret = cbor_encode_text_stringz(&options, "clientPin");
                if (ret == CborNoError) {
                    // NOT [yet] capable of verifying user
                    ret = cbor_encode_boolean(&options, false);
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
        NRF_LOG_DEBUG("Encoding failed");
        return CTAP1_ERR_OTHER;
    }

    // エンコードされたバッファのサイズを設定
    *encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    NRF_LOG_DEBUG("Encoding success (%d bytes)", *encoded_buff_size);
    return CTAP1_ERR_SUCCESS;
}
