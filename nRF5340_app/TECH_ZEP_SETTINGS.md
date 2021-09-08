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
：

static int h_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg)
{
	  // サブツリーに登録されているキーが読み込まれた時に実行されます。
    LOG_DBG("h_set key[%s] len[%d]", log_strdup(key), (int)len);
    int read_len = read_cb(cb_arg, mc, sizeof(mc));
    if (read_len < 0) {
        LOG_ERR("Failed to read from storage: read_cb returns %d", read_len);
    }
    return 0;
}

static int h_commit(void)
{
    // サブツリーに登録されている全てのキーが読み込まれた時に実行されます。
    LOG_DBG("h_commit called");
    return 0;
}

struct settings_handler my_conf = {
    .name = "foo",
    .h_set = h_set,
    .h_commit = h_commit,
};

void app_main_settings_register(void)
{
    settings_register(&my_conf);
}
```


#### データの参照

後報

#### データの登録

後報

#### データの削除

ひとたびサブツリーに登録したキーは、`settings_delete`により消去されるまで、サブツリーに残ります。<br>
注意点として、サブツリー全体を消去するAPIはないため、含まれているキーをすべて検索し、逐一消去する必要があります。

サンプルコードは以下になります。

```
//
// nRF5340_app/secure_device_app/src/app_main.c
//
#include <settings/settings.h>
：
void app_main_button_1_pressed(void)
{
    LOG_DBG("Button 2 pushed");

    int ret = settings_delete("foo/sample/model");
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
# キー`sample/model`を消去します。
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.042,053] <inf> bt_hci_core: No ID address. App must call settings_load()
[00:00:00.042,175] <dbg> app_main.h_set: h_set key[sample/model] len[13]
[00:00:00.044,311] <dbg> app_main.h_commit: h_commit called
：
[00:00:04.495,086] <dbg> app_main.app_main_button_1_pressed: Button 2 pushed
[00:00:04.495,422] <inf> app_main: settings_delete done
#
# `settings_delete`の実行後にリセットを実施。
# `settings_delete`の実行時に指定したキーが、サブツリーから消去されています。
#
*** Booting Zephyr OS build v2.6.0-rc1-ncs1-3-g0944459b5b62  ***
：
[00:00:00.042,053] <inf> bt_hci_core: No ID address. App must call settings_load()
[00:00:00.044,311] <dbg> app_main.h_commit: h_commit called
：
```
