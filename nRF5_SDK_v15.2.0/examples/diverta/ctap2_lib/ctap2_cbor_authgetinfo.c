/* 
 * File:   ctap2_cbor_authgetinfo.c
 * Author: makmorit
 *
 * Created on 2018/12/24, 9:32
 */
#include "sdk_common.h"

#include <stdbool.h>
#include "cbor.h"
#include "fido_common.h"

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

#define CTAP_MAX_MESSAGE_SIZE       1024

//
// aaguid（仮の値です）
//
static uint8_t CTAP_AAGUID[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

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
            ret = cbor_encode_int(&map, CTAP_MAX_MESSAGE_SIZE);
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

bool ctap2_cbor_authgetinfo_response_message(uint8_t *response_buffer, size_t *response_length)
{
    // 作業領域初期化
    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセット
    memset(response_buffer, 0x00, *response_length);
    uint8_t *encoded_buff = response_buffer + 1;
    size_t encoded_buff_size = *response_length - 1;
    
    // CBORエンコーダー初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);

    // CBORエンコード実行
    if (encode_authgetinfo_response_message(&encoder) == false) {
        // １バイトめにエラーコード
        // （CTAP1_ERR_OTHER）をセット
        NRF_LOG_DEBUG("encode_authgetinfo_response_message failed");
        response_buffer[0] = CTAP1_ERR_OTHER;
        *response_length = 1;
        return false;
    }

    // １バイトめにステータスコード
    // （CTAP1_ERR_SUCCESS）をセット
    response_buffer[0] = CTAP1_ERR_SUCCESS;
    *response_length = cbor_encoder_get_buffer_size(&encoder, encoded_buff) + 1;
    return true;
}
