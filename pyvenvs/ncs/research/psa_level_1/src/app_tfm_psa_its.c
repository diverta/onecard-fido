/* 
 * File:   app_tfm_psa_its.c
 * Author: makmorit
 *
 * Created on 2021/04/22, 12:21
 */
#include <zephyr.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>

#include "psa/error.h"
#include "psa/internal_trusted_storage.h"

#include "app_tfm_psa_its.h"

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(app_tfm_psa_its);

// for debug data
#define LOG_DEBUG_ITS_DATA      false

//
// for Internal Trusted Storage
//
static uint8_t m_work_buf[32];

bool app_tfm_psa_its_data_read(uint16_t record_id, uint8_t *record_data, size_t *record_size, bool *is_exist)
{
    // 引数のフラグを初期化
    *is_exist = false;

    // メタデータ格納用
    struct psa_storage_info_t p_info;
    psa_storage_uid_t uid = record_id;

    // Protected Storage のメタデータを取得
    memset(&p_info, 0, sizeof(p_info));
    psa_status_t status = psa_its_get_info(uid, &p_info);
    if (status == PSA_ERROR_DOES_NOT_EXIST) {
        return true;

    } else if (status == PSA_SUCCESS) {
        // Internal Trusted Storage から読込み
        size_t data_offset = 0;
        size_t data_length;
        status = psa_its_get(uid, data_offset, p_info.size, (void *)m_work_buf, &data_length);
        if (status == PSA_SUCCESS) {
#if LOG_DEBUG_ITS_DATA
            // for debug
            LOG_HEXDUMP_DBG(m_work_buf, p_info.size, "psa_its_get");
#endif
            // レコードからデータを抽出
            //   version: 2バイト
            //   size:    2バイト
            //   data:    可変長
            uint16_t *p_size = (uint16_t *)(m_work_buf + 2);
            *record_size = *p_size;
            memcpy(record_data, m_work_buf + 4, *p_size);
            memset(m_work_buf, 0, sizeof(m_work_buf));

            // 引数のフラグに設定
            *is_exist = true;
            return true;
        }

        // 異常終了時は false を戻す
        LOG_ERR("psa_its_get returns %d", status);
        return false;
    }

    // 異常終了時は false を戻す
    LOG_ERR("psa_its_get_info returns %d", status);
    return false;
}

bool app_tfm_psa_its_data_write(uint16_t record_id, uint8_t *record_data, size_t record_size)
{
    psa_storage_uid_t uid = record_id;
    uint16_t version = PSA_ITS_RECORD_VERSION;

    // 引数のデータからレコードを生成
    //   version: 2バイト
    //   size:    2バイト
    //   data:    可変長
    memcpy(m_work_buf, (uint8_t *)(&version), sizeof(uint16_t));
    memcpy(m_work_buf + 2, (uint8_t *)(&record_size), sizeof(uint16_t));
    memcpy(m_work_buf + 4, record_data, record_size);
    psa_status_t status = psa_its_set(uid, record_size + 4, (void *)m_work_buf, PSA_STORAGE_FLAG_NONE);
    if (status == PSA_SUCCESS) {
        return true;
    }

    // 異常終了時は false を戻す
    LOG_ERR("psa_its_set returns %d", status);
    return false;
}
