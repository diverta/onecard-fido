/* 
 * File:   app_tfm_psa_its.h
 * Author: makmorit
 *
 * Created on 2021/04/22, 12:21
 */
#ifndef APP_TFM_PSA_ITS_H
#define APP_TFM_PSA_ITS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// レコードバージョン
//
#define PSA_ITS_RECORD_VERSION          (0x00000001)

//
// レコードUID
//
#define PSA_ITS_TEST_RECORD_KEY         (0x0000AFFF)

//
// 関数群
//
bool        app_tfm_psa_its_data_read(uint16_t record_id, uint8_t *record_data, size_t *record_size, bool *is_exist);
bool        app_tfm_psa_its_data_write(uint16_t record_id, uint8_t *record_data, size_t record_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_TFM_PSA_ITS_H */
