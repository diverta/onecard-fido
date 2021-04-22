/* 
 * File:   app_tfm_psa_ps.h
 * Author: makmorit
 *
 * Created on 2021/04/22, 12:21
 */
#ifndef APP_TFM_PSA_PS_H
#define APP_TFM_PSA_PS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// レコードバージョン
//
#define PSA_PS_RECORD_VERSION           (0x00000001)

//
// レコードUID
//
#define PSA_PS_TEST_RECORD_KEY          (0x0000BFFF)

//
// 関数群
//
bool        app_tfm_psa_ps_data_read(uint16_t record_id, uint8_t *record_data, size_t *record_size, bool *is_exist);
bool        app_tfm_psa_ps_data_write(uint16_t record_id, uint8_t *record_data, size_t record_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_TFM_PSA_PS_H */
