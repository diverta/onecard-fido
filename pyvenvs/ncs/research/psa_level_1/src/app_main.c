/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/21, 11:20
 */
#include <zephyr.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>

#include "tfm_ns_interface.h"

#include "psa/error.h"
#include "psa/protected_storage.h"
#include "psa/internal_trusted_storage.h"

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(app_main);

// データ内容
typedef struct {
    uint32_t uid;
    uint16_t version;
    uint8_t  data[16];
} PSA_PS_DATA_T;

typedef struct {
    uint32_t uid;
    uint16_t version;
    uint8_t  data[16];
} PSA_ITS_DATA_T;

// データID
#define SAMPLE_PS_DATA_UID      0x5595DA7A
#define SAMPLE_ITS_DATA_UID     0x6175DA7A

// データID（64-bit UID）を保持
static psa_storage_uid_t sample_ps_data_uid  = SAMPLE_PS_DATA_UID;
static psa_storage_uid_t sample_its_data_uid = SAMPLE_ITS_DATA_UID;

// デフォルトのデータ
static PSA_PS_DATA_T m_ps_data = {
    .uid = SAMPLE_PS_DATA_UID,
    .version = 1,
    .data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
};

static PSA_ITS_DATA_T m_its_data = {
    .uid = SAMPLE_ITS_DATA_UID,
    .version = 1,
    .data = {20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35}
};

//
// for Private key
//
#include "psa/crypto.h"
#include "psa/crypto_compat.h"
#include "psa/crypto_types.h"
#include "psa/crypto_values.h"

#define PRIVATE_KEY_prime256v1 \
0xf1, 0xb7, 0x14, 0x23, 0x43, 0x40, 0x2f, 0x3b, 0x5d, 0xe7, 0x31, 0x5e, 0xa8, \
0x94, 0xf9, 0xda, 0x5c, 0xf5, 0x03, 0xff, 0x79, 0x38, 0xa3, 0x7c, 0xa1, 0x4e, \
0xb0, 0x32, 0x86, 0x98, 0x84, 0x50

void research_private_key(void)
{
    // 秘密鍵のテストデータ
    static const uint8_t private_key_256[] = {PRIVATE_KEY_prime256v1};

    psa_key_type_t  key_type        = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP256R1);
    psa_algorithm_t key_alg         = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    uint8_t        *private_key     = private_key_256;
    size_t          private_key_len = sizeof(private_key_256);

    psa_key_handle_t     key_handle;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();

    // Setup the key policy for private key
    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_SIGN);
    psa_set_key_algorithm(&key_attributes, key_alg);
    psa_set_key_type(&key_attributes, key_type);

    psa_status_t crypto_result = psa_import_key(&key_attributes, private_key, private_key_len, &key_handle);
    LOG_INF("Private key import: psa_import_key returns %d", crypto_result);
    if (crypto_result != PSA_SUCCESS) {
        // TODO
    }
}

//
// for Protected Storage & Internal Trusted Storage
//
void research_secure_storage(void)
{
    // メタデータ格納用
    struct psa_storage_info_t p_info;

    // Protected Storage
    memset(&p_info, 0, sizeof(p_info));
    psa_status_t status = psa_ps_get_info(sample_ps_data_uid, &p_info);
    if (status == PSA_ERROR_DOES_NOT_EXIST) {
        // データを新規生成
	status = psa_ps_set(sample_ps_data_uid, sizeof(PSA_PS_DATA_T), (void *)&m_ps_data, PSA_STORAGE_FLAG_NONE);
        LOG_INF("Protected Storage: created");

    } else if (status == PSA_SUCCESS) {
        // データ内容を表示
        LOG_INF("Protected Storage: metadata (capacity=%d, size=%d)", p_info.capacity, p_info.size);
        PSA_PS_DATA_T ps_data;
        size_t data_offset = 0;
        size_t data_length;
        status = psa_ps_get(sample_ps_data_uid, data_offset, sizeof(PSA_PS_DATA_T), (void *)&ps_data, &data_length);
        if (status == PSA_SUCCESS) {
            LOG_HEXDUMP_DBG(&ps_data, data_length, "ps_data");
        } else {
            LOG_ERR("Protected Storage: psa_ps_get returns %d", status);
        }

    } else {
        LOG_ERR("Protected Storage: psa_ps_get_info returns %d", status);
    }

    // Internal Trusted Storage
    memset(&p_info, 0, sizeof(p_info));
    status = psa_its_get_info(sample_its_data_uid, &p_info);
    if (status == PSA_ERROR_DOES_NOT_EXIST) {
        // データを新規生成
	status = psa_its_set(sample_its_data_uid, sizeof(PSA_ITS_DATA_T), (void *)&m_its_data, PSA_STORAGE_FLAG_NONE);
        LOG_INF("Internal Trusted Storage: created");

    } else if (status == PSA_SUCCESS) {
        // データ内容を表示
        LOG_INF("Internal Trusted Storage: metadata (capacity=%d, size=%d)", p_info.capacity, p_info.size);
        PSA_ITS_DATA_T its_data;
        size_t data_offset = 0;
        size_t data_length;
        status = psa_its_get(sample_its_data_uid, data_offset, sizeof(PSA_ITS_DATA_T), (void *)&its_data, &data_length);
        if (status == PSA_SUCCESS) {
            LOG_HEXDUMP_DBG(&its_data, data_length, "its_data");
        } else {
            LOG_ERR("Internal Trusted Storage: psa_its_get returns %d", status);
        }

    } else {
        LOG_ERR("Internal Trusted Storage: psa_its_get_info returns %d", status);
    }
    
    LOG_INF("research terminated.");
}

void app_main(void)
{
    // 調査用
    research_private_key();
    research_secure_storage();
}
