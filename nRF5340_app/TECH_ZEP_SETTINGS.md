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

|キーワード|名称|内容|
|:--|:-|:-|
|`name`|サブツリー名称||
|`h_set`|キー参照callback   |サブツリーに登録されているキーが読み込まれた時に実行される関数|
|`h_commit`|参照完了callback|サブツリーに登録されている全てのキーが読み込まれた時に実行される関数|

nRF5340アプリケーションのコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_settings.c
//
#include <settings/settings.h>

// サブツリー設定
static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg);
static int h_commit(void);
static struct settings_handler app_conf = {
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

`h_set`の実装コードです。<br>
サブツリーに登録されているキーごとに呼び出され、関数内でキーに対応するデータを読み込むことができます。

```
static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    ：
    // バッファ長を上限として、検索対象のデータを読込
    size_t max = (len > sizeof(settings_buf) ? sizeof(settings_buf) : len);
    int read_len = read_cb(cb_arg, settings_buf, max);
    ：
}
```

`h_commit`の実装コードです。<br>
サブツリーに登録されている全てのキーについて、`h_set`呼び出しが完了すると実行されます。<br>
関数内ではロード完了時の処理を記述します。

```
static int h_commit(void)
{
    // 検索キーをクリア
    settings_key_to_find = NULL;
    return 0;
}
```

#### データの参照

nRF5340アプリケーションでは、登録データ参照用の関数として、`app_settings_find`を用意しています。<br>
関数引数には登録用キーと、登録データ読込用のバッファを指定します。

```
//
// 呼び出し例
//
#include "app_settings.h"

APP_SETTINGS_KEY key = {0xBFFD, 0xBFED, false, 0};
app_settings_find(&key, c, &s);

//
// nRF5340_app/secure_device_app/src/app_settings.c
//
bool app_settings_find(APP_SETTINGS_KEY *key, void *value, size_t *value_size)
{
    // データ格納領域を初期化
    memset(settings_buf, 0, sizeof(settings_buf));
    settings_buf_size = 0;

    // サブツリーをロード
    //   検索対象データが settings_buf に
    //   格納されます。
    if (app_settings_load(key) == false) {
        return false;
    }
    ：
}
```

登録用キー`APP_SETTINGS_KEY`は以下のような内容になっています。<br>
具体的にはnRF52840アプリケーションで使用していた[ファイルID／レコードキー](https://github.com/diverta/onecard-fido/blob/master/nRF52840_app/examples/diverta/plat_lib/fido_flash.h)と同様の想定です。

|キーワード|名称|内容|
|:--|:-|:-|
|`file_id`|ファイルID|データを業務単位にまとめるためのID<br>（業務共通／FIDO／PIV／OpenPGP）|
|`record_key`|レコードキー|データを識別するためのキー|
|`serial`|連番|レコードキー配下のデータを識別する番号<br>（同一レコードID配下に複数のデータを登録したい場合に使用）|
|`use_serial`|連番使用|連番を使用する場合`true`、使用しない場合`false`|

サブツリーに登録されているデータを参照するために、内部でZephyrのAPI`settings_load_subtree`を呼び出しています。[注1]<br>
引数には、前述のサブツリー名称`app`（`app_conf.name`）を指定します。

同時に、モジュール変数`settings_key_to_find`に、検索対象のキーを設定しておきます。

```
static bool app_settings_load(APP_SETTINGS_KEY *key)
{
    // キー名を生成
    create_app_settings_key(key, settings_key);

    if (settings_name_steq(settings_key, app_conf.name, &settings_key_to_find) == 0) {
        // 検索対象のキーが抽出できなかった場合は何も行わない
        return false;
    }

    // サブツリーをロード
    int ret = settings_load_subtree(app_conf.name);
    if (ret != 0) {
        LOG_ERR("settings_load_subtree returns %d", ret);
        return false;
    }

    return true;
}
```

前述の通り、`settings_load_subtree`を呼び出すと、登録されているキーごとに`h_set`が１回ずつ呼び出されます。<br>
`h_set`では、呼び出しごとにキーが`settings_key_to_find`と同じかどうか判定し、同じであれば`settings_buf`に検索対象のデータを読込みます。

```
// 登録データの読込用バッファ
static uint8_t settings_buf[128];

// app_settings_loadで指定された検索キーを保持
static const char *settings_key_to_find = NULL;

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    ：
    // 検索対象かどうかを判定
    if (settings_key_to_find == NULL || strcmp(key, settings_key_to_find) != 0) {
        return 0;
    }

    // バッファ長を上限として、検索対象のデータを読込
    size_t max = (len > sizeof(settings_buf) ? sizeof(settings_buf) : len);
    int read_len = read_cb(cb_arg, settings_buf, max);
    if (read_len < 0) {
        LOG_ERR("Failed to read from storage: read_cb returns %d", read_len);
        return read_len;
    }
    ：
}
```

`app_settings_find`は、最後に検索対象のデータを`settings_buf`から引数`value`の領域にコピーし、呼び出し先に戻ります。<br>
検索対象データ長は`value_size`に設定されます。<br>
（検索対象のデータが見つからなかった場合は、`value_size`に`0`が設定されます）

```
bool app_settings_find(APP_SETTINGS_KEY *key, void *value, size_t *value_size)
{
    ：
    // ロードしたデータをコピー
    memcpy(value, settings_buf, settings_buf_size);
    *value_size = settings_buf_size;
    return true;
}
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
