/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/21, 11:20
 */
#include <zephyr.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>

#include "app_tfm_psa_its.h"
#include "app_tfm_psa_ps.h"

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(app_main);

// データ内容
static uint8_t m_ps_data[16] = {20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35};
static uint8_t m_its_data[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void research_secure_storage(void)
{
    // 一時変数
    size_t record_size;
    bool   is_exist;

    // Protected Storage
    if (app_tfm_psa_ps_data_read(PSA_PS_TEST_RECORD_KEY, m_ps_data, &record_size, &is_exist) == false) {
        return;
    }
    if (is_exist) {
        LOG_INF("Protected Storage: exist");
        LOG_HEXDUMP_DBG(m_ps_data, record_size, "m_ps_data");
    } else {
        record_size = sizeof(m_ps_data);
        if (app_tfm_psa_ps_data_write(PSA_PS_TEST_RECORD_KEY, m_ps_data, record_size) == false) {
            return;
        }
        LOG_INF("Protected Storage: created");
    }

    // Internal Trusted Storage
    if (app_tfm_psa_its_data_read(PSA_ITS_TEST_RECORD_KEY, m_its_data, &record_size, &is_exist) == false) {
        return;
    }
    if (is_exist) {
        LOG_INF("Internal Trusted Storage: exist");
        LOG_HEXDUMP_DBG(m_its_data, record_size, "m_its_data");
    } else {
        record_size = sizeof(m_its_data);
        if (app_tfm_psa_its_data_write(PSA_ITS_TEST_RECORD_KEY, m_its_data, record_size) == false) {
            return;
        }
        LOG_INF("Internal Trusted Storage: created");
    }

    LOG_INF("research terminated.");
}

void app_main(void)
{
    // 調査用
    research_secure_storage();
}
