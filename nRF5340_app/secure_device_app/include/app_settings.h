/* 
 * File:   app_settings.h
 * Author: makmorit
 *
 * Created on 2021/09/08, 15:54
 */
#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

//
// レコードキーを保持
//   ファイルID
//   レコードID
//   連番（同一レコードID配下に複数のデータが存在する場合に使用）
//
typedef struct {
    uint16_t    file_id;
    uint16_t    record_key;
    bool        use_serial;
    uint16_t    serial;
} APP_SETTINGS_KEY;

//
// 関数群
//
void        app_settings_initialize(void);
bool        app_settings_save(APP_SETTINGS_KEY *key, void *value, size_t value_size);
bool        app_settings_find(APP_SETTINGS_KEY *key, void *value, size_t *value_size);
bool        app_settings_delete(APP_SETTINGS_KEY *key);

#ifdef __cplusplus
}
#endif

#endif /* APP_SETTINGS_H */
