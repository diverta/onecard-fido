/* 
 * File:   app_settings.c
 * Author: makmorit
 *
 * Created on 2021/09/08, 15:54
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <settings/settings.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_settings);

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    // TODO: 仮の実装です。
    return 0;
}

static int h_commit(void)
{
    // TODO: 仮の実装です。
    return 0;
}

struct settings_handler app_conf = {
    .name     = "app",
    .h_set    = h_set,
    .h_commit = h_commit,
};

void app_settings_initialize(void)
{
    // 業務処理で使用する永続化用のサブツリーを登録
    settings_register(&app_conf);

    // 永続化機能を初期化
    if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
        settings_load();
    }
}
