# データの永続化について

最新更新日：2021/09/23

## 概要

Zephyrプラットフォームにおけるデータの永続化に関する技術情報を、補足的に掲載しています。

## Settingsサブシステム

データ永続化は、Zephyrプラットフォーム内に組込まれている`Settings`というサブシステムを使用して行います。

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
CONFIG_APP_SETTINGS_BUFFER_SIZE=1024 [注1]
：
```

[注1] 読込用バッファ長（１レコードあたり最大バイト数）。デフォルトは128バイトですが、`prj.conf`に記述することで拡張が可能です。

## 永続化用関数群

nRF5340アプリケーションでは、データ永続化のために、下記のような関数を実装しています。

|キーワード|名称|内容|
|:--|:-|:-|
|`app_settings_initialize`|永続化機能初期化|`Settings`サブシステムに、サブツリー`"app"`を登録します。|
|`app_settings_save`|データ登録|データにキーを付与し、サブツリーに登録|
|`app_settings_find`|データ参照|指定のキーに対応する登録データを取得|
|`app_settings_search`|データ検索|指定のキー配下に属する登録データのうち、指定条件に合致する<br>データのみを取得[注2]|
|`app_settings_delete`|データ削除|指定のキーに対応する登録データを削除|
|`app_settings_delete_multi`|データ複数件削除|指定のキー配下に属する全ての登録データを削除|

[注2] 指定条件に合致するかどうかをチェックするための関数を別途用意し、その関数の参照を`app_settings_search`の引数として渡す仕様としております。

#### 永続化機能の初期化

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

`h_set`の実装コード例です。<br>
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

`h_commit`の実装コード例です。<br>
サブツリーに登録されている全てのキーについて、`h_set`呼び出しが完了すると実行されます。<br>
関数内ではロード完了時の処理を記述します。

```
static int h_commit(void)
{
    // 検索キーをクリア
    settings_key_to_find   = NULL;
    settings_key_to_delete = NULL;
    return 0;
}
```

#### データの登録

nRF5340アプリケーションでは、データ登録用の関数として、`app_settings_save`を用意しています。<br>
関数引数には登録用キーと、登録データ格納用のバッファを指定します。

```
//
// 呼び出し例
//
#include "app_settings.h"

APP_SETTINGS_KEY key1 = {0xBFFE, 0xBFEE, false, 0};
char *m1 = "sample value 01";
app_settings_save(&key1, m1, strlen(m1));
```

データをサブツリーに登録するために、内部でZephyrのAPI`settings_save_one`を呼び出しています。

```
//
// nRF5340_app/secure_device_app/src/app_settings.c
//
bool app_settings_save(APP_SETTINGS_KEY *key, void *value, size_t value_size)
{
    // キー名を生成
    create_app_settings_key(key, settings_key);

    // サブツリーにデータを登録
    int ret = settings_save_one(settings_key, value, value_size);
    :
}
```

`app_settings_save`で登録したデータを、後に業務処理で参照したい場合は、後述関数`app_settings_find`を実行します。

#### データの参照

nRF5340アプリケーションでは、登録データ参照用の関数として、`app_settings_find`を用意しています。<br>
関数引数には登録用キーと、登録データ読込用のバッファを指定します。

```
//
// 呼び出し例
//
#include "app_settings.h"

APP_SETTINGS_KEY key = {0xBFFD, 0xBFED, false, 0};
bool exist;
size_t size;
app_settings_find(&key, &exist, data, &size);
```

登録用キー`APP_SETTINGS_KEY`は以下のような内容になっています。<br>
具体的にはnRF52840アプリケーションで使用していた[ファイルID／レコードキー](https://github.com/diverta/onecard-fido/blob/master/nRF52840_app/examples/diverta/plat_lib/fido_flash.h)と同様の想定です。

|キーワード|名称|内容|
|:--|:-|:-|
|`file_id`|ファイルID|データを業務単位にまとめるためのID<br>（業務共通／FIDO／PIV／OpenPGP）|
|`record_key`|レコードキー|データを識別するためのキー|
|`serial`|連番|レコードキー配下のデータを識別する番号<br>（同一レコードID配下に複数のデータを登録したい場合に使用）|
|`use_serial`|連番使用|連番を使用する場合`true`、使用しない場合`false`|

データ参照の際には、サブツリーに登録されているキーをすべて参照する必要があるため、`app_settings_load`を経由して行います。

```
//
// nRF5340_app/secure_device_app/src/app_settings.c
//
bool app_settings_find(APP_SETTINGS_KEY *key, bool *exist, void *value, size_t *value_size)
{
    return app_settings_search(key, exist, value, value_size, NULL);
}

bool app_settings_search(APP_SETTINGS_KEY *key, bool *exist, void *value, size_t *value_size, bool (*_condition_func)(const char *key, void *data, size_t size))
{
    :
    // サブツリーをロード
    //   検索対象データがサブツリー内に存在する場合、
    //   データが settings_buf に格納され、
    //   データ長が settings_buf_size に設定されます。
    if (app_settings_load(key, &settings_key_to_find) == false) {
        return false;
    }
    :
}
```

`app_settings_load`内では、サブツリーに登録されているデータを参照するために、ZephyrのAPI`settings_load_subtree`を呼び出しています。[注3]<br>
引数には、前述のサブツリー名称`app`（`app_conf.name`）を指定します。

同時に、モジュール変数`settings_key_to_find`に、検索対象のキーを設定しておきます。

```
static bool app_settings_load(APP_SETTINGS_KEY *key, const char **key_to_find)
{
    // キー名を生成
    create_app_settings_key(key, settings_key);

    if (settings_name_steq(settings_key, app_conf.name, key_to_find) == 0) {
        // 検索対象のキーが抽出できなかった場合は何も行わない
        return false;
    }

    // サブツリーをロード
    int ret = settings_load_subtree(app_conf.name);
    :
}
```

前述の通り、`settings_load_subtree`を呼び出すと、登録されているキーごとに`h_set`が１回ずつ呼び出されます。<br>
`h_set`では、呼び出しごとにキーが`settings_key_to_find`と同じかどうか判定し、同じであれば`settings_buf`に検索対象のデータを読込みます。

```
// 登録データの読込用バッファ
static uint8_t settings_buf[CONFIG_APP_SETTINGS_BUFFER_SIZE];

// app_settings_loadで指定された検索キーを保持
static const char *settings_key_to_find = NULL;

static int find_setting(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    :
    // キーが検索対象でない場合は終了
    if (strncmp(key, settings_key_to_find, strlen(settings_key_to_find)) != 0) {
        return 0;
    }

    // バッファ長を上限として、検索対象のデータを読込
    size_t max = (len > sizeof(settings_buf) ? sizeof(settings_buf) : len);
    int read_len = read_cb(cb_arg, settings_buf, max);
    :
}

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    ：
    // キーが検索対象であれば、検索対象のデータを読込
    if (settings_key_to_find != NULL && strcmp(key, settings_key_to_find) == 0) {
        return find_setting(key, len, read_cb, cb_arg);
    }
    ：
}
```

`app_settings_find`内部で呼び出されている`app_settings_search`は、最後に検索対象のデータを`settings_buf`から引数`value`の領域にコピーし、呼び出し先に戻ります。<br>
検索対象データ長は`value_size`に設定されます。<br>
（検索対象のデータが見つからなかった場合は、`value_size`に`0`が設定されます）

検索対象のデータが見つかった場合は、`exist`に`true`が設定されます。<br>
（見つからなかった場合は`false`が設定されます）

```
bool app_settings_search(APP_SETTINGS_KEY *key, bool *exist, void *value, size_t *value_size, bool (*_condition_func)(const char *key, void *data, size_t size))
{
    :
    // ロードしたデータをコピー
    if (settings_buf_size > 0) {
        memcpy(value, settings_buf, settings_buf_size);
        *exist = true;
    }
    *value_size = settings_buf_size;
    return true;
}
```

[注3]（ご参考）nRF5340アプリケーションでは、ボンディング機能を有効化するため、初期処理において`settings_load`を呼び出すようにしています。このため、`settings_load`実行時、自動的に`settings_load_subtree`が１度呼び出される仕様になっています。

#### データの検索

nRF5340アプリケーションでは、データ検索用関数として、`app_settings_search`を用意しています。<br>
この関数では、指定のキー配下に属する登録データのうち、指定条件に合致するデータのみを取得できます。

関数引数には登録用キーと、登録データ読込用のバッファ、さらにユーザー関数の参照を指定します。<br>
ユーザー関数は、各々の登録データについて、指定条件に合致するかどうかをチェックするよう実装します。

```
//
// 呼び出し例
//
#include "app_settings.h"

bool _condition_func(const char *key, void *data, size_t size)
{
    // サブツリーに登録されているデータについて、
    // 指定条件に合致するかどうかをチェック
    return (strncmp((char *)data, "sample value 123", size) == 0);
}

APP_SETTINGS_KEY key0 = {0xBFFE, 0, false, 0};
app_settings_search(&key0, &exist, buf, &size, _condition_func);
```

データ参照の際には、サブツリーに登録されているキーをすべて参照する必要があるため、`app_settings_load`を経由して行います。

```
//
// nRF5340_app/secure_device_app/src/app_settings.c
//
bool app_settings_search(APP_SETTINGS_KEY *key, bool *exist, void *value, size_t *value_size, bool (*_condition_func)(const char *key, void *data, size_t size))
{
    :
    // 条件判定用関数の参照を保持
    settings_condition_func = _condition_func;

    // サブツリーをロード
    //   検索対象データがサブツリー内に存在する場合、
    //   データが settings_buf に格納され、
    //   データ長が settings_buf_size に設定されます。
    if (app_settings_load(key, &settings_key_to_find) == false) {
        return false;
    }
    :
}
```

`app_settings_load`を経由し呼び出された`h_set`においては、呼び出しごとにキーが`settings_key_to_find`と同じかどうか判定し、さらにユーザー関数（`settings_condition_func`）によりデータが所定条件に合致するかどうかチェックします。<br>
以上のチェックにより、条件に合致したと判定された場合`settings_buf`に検索対象のデータを読込みます。

```
static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    :
    // キーが検索対象であれば、検索対象のデータを読込
    if (settings_key_to_find != NULL) {
        return find_setting(key, len, read_cb, cb_arg);
    }
    :
}

static int find_setting(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    :
    // キーが検索対象でない場合は終了
    if (strncmp(key, settings_key_to_find, strlen(settings_key_to_find)) != 0) {
        return 0;
    }

    // バッファ長を上限として、検索対象のデータを読込
    size_t max = (len > sizeof(settings_buf) ? sizeof(settings_buf) : len);
    int read_len = read_cb(cb_arg, settings_buf, max);
    :
    if (settings_condition_func != NULL) {
        // データ内容を比較し、所定の内容と一致していれば、
        // 読み込んだバイト数を保持
        if ((*settings_condition_func)(settings_buf, read_len)) {
            settings_buf_size = read_len;
        }
        :
}
```

`app_settings_search`は、最後に検索対象のデータを`settings_buf`から引数`value`の領域にコピーし、呼び出し先に戻ります。<br>
検索対象データ長は`value_size`に設定されます。<br>
（検索対象のデータが見つからなかった場合は、`value_size`に`0`が設定されます）

検索対象のデータが見つかった場合は、`exist`に`true`が設定されます。<br>
（見つからなかった場合は`false`が設定されます）

```
bool app_settings_search(APP_SETTINGS_KEY *key, bool *exist, void *value, size_t *value_size, bool (*_condition_func)(const char *key, void *data, size_t size))
{
    :
    // ロードしたデータをコピー
    if (settings_buf_size > 0) {
        memcpy(value, settings_buf, settings_buf_size);
        *exist = true;
    }
    *value_size = settings_buf_size;
    return true;
}
```


#### データの削除

nRF5340アプリケーションでは、登録データ削除用の関数として、`app_settings_delete`を用意しています。<br>
関数引数には削除用キーを指定します。

```
//
// 呼び出し例
//
#include "app_settings.h"

// キーに該当するデータを個別削除
APP_SETTINGS_KEY key1 = {0xBFFE, 0xBFEE, false, 0};
app_settings_delete(&key1);
```

データをサブツリーから削除するために、内部でZephyrのAPI`settings_delete`を呼び出しています。<br>

```
//
// nRF5340_app/secure_device_app/src/app_settings.c
//
bool app_settings_delete(APP_SETTINGS_KEY *key)
{
    // キー名を生成
    create_app_settings_key(key, settings_key);

    // サブツリーから該当データだけを削除
    int ret = settings_delete(settings_key);
    if (ret != 0) {
        LOG_ERR("settings_delete returns %d", ret);
        return false;
    }

#if LOG_SETTINGS_DELETE
    LOG_INF("settings_delete done: key[%s]", log_strdup(settings_key));
#endif

    return true;
}
```

#### データの複数件削除

nRF5340アプリケーションでは、指定のキー配下に属する全ての登録データを削除するための関数として、`app_settings_delete_multi`を用意しています。<br>
関数引数には削除用キーを指定します。

```
//
// 呼び出し例
//
#include "app_settings.h"

// 0xBFFE 配下のデータを全削除
APP_SETTINGS_KEY key2 = {0xBFFE, 0, false, 0};
app_settings_delete(&key2);
```

複数件削除の場合は、サブツリーに登録されているキーをすべて参照する必要があるため、`app_settings_load`を経由して行います。

```
bool app_settings_delete_multi(APP_SETTINGS_KEY *key)
{
    // キー名を生成
    create_app_settings_key(key, settings_key);

    // 指定されたキーの配下に属するデータを
    // サブツリーから全て削除
    if (app_settings_load(key, &settings_key_to_delete) == false) {
        return false;
    }

    return true;
}
```

`app_settings_load`により呼び出された`h_set`内で、サブツリーに登録されているキーを、指定されたキーと比較し、前方一致した場合、そのキーのデータをサブツリーから削除しています。<br>
データをサブツリーから削除するために、ZephyrのAPI`settings_delete`を呼び出します。

```
static int delete_setting(const char *key)
{
    // キーが削除対象でない場合は終了
    if (strncmp(key, settings_key_to_delete, strlen(settings_key_to_delete)) != 0) {
        return 0;
    }

    // 該当キーのデータをサブツリーから削除
    sprintf(settings_key_temp, "%s/%s", app_conf.name, key);
    int rc = settings_delete(settings_key_temp);
    :
}

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    :
    // キーが削除対象であれば、該当キーのデータをサブツリーから削除
    if (settings_key_to_delete != NULL) {
        return delete_setting(key);
    }
    :
}
```
