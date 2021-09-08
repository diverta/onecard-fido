# データの永続化について

最新更新日：2021/09/08

## 概要

Zephyrプラットフォームにおけるデータの永続化に関する技術情報を、補足的に掲載しています。

## Settingsサブシステム

データ永続化は、Zephyrプラットフォーム内に組込まれている`Settings`というサブシステムを使用して行います。<br>
以下は、主にアプリケーション・プログラムの機能についての記述になります。

#### 定義体

`Settings`サブシステムを使用する場合は、アプリケーション定義`prj.conf`に、下記のようなエントリーを追加する必要があります。

```
//
// nRF5340_app/secure_device_app/prj.conf
//
#
# for Flash ROM
#
CONFIG_FLASH=y
CONFIG_FLASH_MAP=y
CONFIG_FLASH_PAGE_LAYOUT=y
CONFIG_FLASH_LOG_LEVEL_DBG=n

# enable Non-volatile Storage in flash
CONFIG_NVS=y

# enable user data setting
CONFIG_SETTINGS=y
：
```

#### サブツリーの登録

まずは`Settings`サブシステムに、サブツリーというオブジェクトを登録します。<br>
このサブツリーの配下に、任意のデータを、キーを付与して永続化します。

サブツリー登録は、`settings_register`というAPIを使用します。<br>
サブツリーが存在しない場合は、自動的に新規生成されます。

以下の内容を登録します。
- `name`：サブツリー名称
- `h_set`：サブツリーに登録されているキーが読み込まれた時に実行される関数
- `h_commit`：サブツリーに登録されている全てのキーが読み込まれた時に実行される関数

サンプルコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_main.c
//
#include <settings/settings.h>

// 登録データの読込用バッファ
static uint8_t settings_buf[128];

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    // サブツリーに登録されているキーが読み込まれた時に実行されます。
    int read_len = read_cb(cb_arg, settings_buf, sizeof(settings_buf));
    if (read_len < 0) {
        LOG_ERR("Failed to read from storage: read_cb returns %d", read_len);
    }
    LOG_INF("h_set called: key[%s] value[%s]", log_strdup(key), log_strdup(settings_buf));
    return 0;
}

static int h_commit(void)
{
    // サブツリーに登録されている全てのキーが読み込まれた時に実行されます。
    LOG_INF("h_commit called");
    return 0;
}

struct settings_handler my_conf = {
    .name = "foo",
    .h_set = h_set,
    .h_commit = h_commit,
};

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
```

#### データの参照

登録されているデータを参照するためには、API`settings_load_subtree`を呼び出します。[注1]<br>
引数には、前述のサブツリー名称`app`を指定します。

サンプルコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_main.c
//
void app_main_button_1_pressed(void)
{
    LOG_DBG("Button 2 pushed");
    // サブツリーをロード
    int ret = settings_load_subtree("app");
    if (ret != 0) {
        LOG_ERR("settings_load_subtree returns %d", ret);
        return;
    }
    LOG_INF("settings_load_subtree done");
}
```

以下はサンプルコード実行時のログになります。

サブツリーをロードすると、登録されているキーごとに`h_set`が１回ずつ呼び出され、バッファ`settings_buf`に登録データがセットされます。<br>
全てのキーについて`h_set`の実行が完了すると、`h_commit`が呼び出され、ロード完了を通知します。

```
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:16.498,596] <dbg> app_main.app_main_button_1_pressed: Button 2 pushed
[00:00:16.498,687] <inf> app_settings: h_set called: key[sample] value[sample value]
[00:00:16.499,114] <inf> app_settings: h_commit called
[00:00:16.499,114] <inf> app_main: settings_load_subtree done
```

[注1] nRF5340アプリケーションでは、ボンディング機能を有効化するため、初期処理において`settings_load`を呼び出すようにしています。このため、`settings_load`実行時、自動的に`settings_load_subtree`が１度呼び出される仕様になっています。

#### データの登録

データをサブツリーに登録するためには、API`settings_save_one`を呼び出します。<br>
引数には、データのキー名称と、データおよびデータ長を指定します。

サンプルコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_main.c
//
const char *mk = "app/sample";
const char *m = "sample value";

void app_main_button_pressed_short(void)
{
    LOG_DBG("Short pushed");
    // サブツリーにデータを登録
    int ret = settings_save_one(mk, m, strlen(m));
    if (ret != 0) {
        LOG_ERR("settings_save_one returns %d", ret);
        return;
    }
    LOG_INF("settings_save_one done: key[%s] value[%s]", log_strdup(mk), log_strdup(m));
}
```

以下はサンプルコード実行時のログになります。

```
#
# データがサブツリーに未登録の状態で起動
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.049,407] <inf> app_settings: h_commit called
#
# `settings_save_one`を実行し、データをサブツリーに登録
#
[00:00:06.178,527] <dbg> app_main.app_main_button_pressed_short: Short pushed
[00:00:06.179,565] <inf> app_main: settings_save_one done: key[app/sample] value[sample value]
#
# `settings_save_one`の実行後にリセットを実施
# `settings_save_one`で登録したデータが、`h_set`で参照されています。
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.042,144] <inf> app_settings: h_set called: key[sample] value[sample value]
[00:00:00.049,560] <inf> app_settings: h_commit called

```

#### データの削除

ひとたびサブツリーに登録したキーは、`settings_delete`により消去されるまで、サブツリーに残ります。<br>
注意点として、サブツリー全体を消去するAPIはないため、含まれているキーをすべて検索し、逐一消去する必要があります。

サンプルコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_main.c
//
const char *mk = "app/sample";

void app_main_button_1_pressed(void)
{
    LOG_DBG("Button 2 pushed");
    int ret = settings_delete(mk);
    if (ret != 0) {
        LOG_ERR("settings_delete returns %d", ret);
        return;
    }
    LOG_INF("settings_delete done");
}
```

以下はサンプルコード実行時のログになります。

```
#
# キー`app/sample`を消去します。
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.042,114] <inf> app_settings: h_set called: key[sample] value[sample value 2]
[00:00:00.049,285] <inf> app_settings: h_commit called
：
[00:00:11.556,823] <dbg> app_main.app_main_button_1_pressed: Button 2 pushed
[00:00:11.557,312] <inf> app_main: settings_delete done
#
# `settings_delete`の実行後にリセットを実施。
# キー`app/sample`がサブツリーから消去されたため、`h_set`で参照されなくなりました。
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.049,407] <inf> app_settings: h_commit called
：
```
