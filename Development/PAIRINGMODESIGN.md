# ペアリングモード標識対応について

## 概要

ペアリングモードであることを示すキャラクタリスティック（以降「ペアリングモード標識」と称します。）を、BLE U2Fサービスに追加し、One CardがペアリングモードであることをU2F管理ツールから認識できるようにします。

（[Issue #58](https://github.com/diverta/onecard-fido/issues/58) ご参照）

## 目的

現状のOne Cardは、ペアリングモードの時に、U2F管理ツールからヘルスチェックなどの処理を実行できてしまうようです。

他方、ペアリングモードは、本来ペアリングのための専用モードであるという観点から見た場合、ペアリング実行以外の機能が実行できないようにすべきです。

One Cardがペアリングモードに移行している場合は、U2F管理ツールからペアリング実行以外の処理を禁止するようにします。

## 対応方針

各プログラムで、以下の制御を行うようにします。

- One Card側では、ペアリングモードではペアリングモード標識を見せ、非ペアリングモード（＝FIDO機能等を実行する業務処理モード）ではペアリングモード標識を見せないようにします。

- U2F管理ツール側では、デバイス接続処理時にペアリングモード標識がディスカバーされた場合、ペアリングモードであると判定するとともに、他のU2Fキャラクタリスティックのディスカバーを行わないようにします。

- さらにU2F管理ツール側では、ペアリングモード時にOne Cardとの通信を伴う機能のボタンが押下された場合、下記のメッセージをテキストエリアに表示して処理を中止させるようにします。
```
ペアリングモードでは、ペアリング実行以外の機能は使用できません。
ペアリングモードを解除してから、機能を再度実行してください。
```

## 注意事項

### ペアリングモード標識の独自性

本件で追加するペアリングモード標識は、Bluetooth SIGの標準で定義されているキャラクタリスティックではありません。
また、FIDOアライアンスにより定義されているキャラクタリスティックでもないです。

上記、定義済みのキャラクタリスティックのうち、ペアリングモード標識と用途が似通っているものを、ペアリングモード標識として流用する方法もあるかと思われますが、後日にその流用元キャラクタリスティックをOne Card上で使用することになった場合、UUIDが重複し使用不能になる等の不具合が発生してしまいます。

したがってペアリングモード標識は、非標準の独自キャラクタリスティックとして独自定義したうえで使用する必要があるかと存じます。

### ペアリングモード標識のオリジナルUUID

独自定義キャラクタリスティックのUUIDは、既存のUUIDとかぶる可能性がない、オリジナルUUIDを生成する必要があるかと思われます。

このため、macOSの`uuidgen`コマンドを使用する方法を採用したいと考えます。
下記実行結果`98439EE6-776B-401C-880C-682FBDDD8E32`を、オリジナルUUIDとして使用することといたします。

```
MacBookPro-makmorit-jp:~ makmorit$ uuidgen
98439EE6-776B-401C-880C-682FBDDD8E32
```

## nRF52側の対応内容

ペアリングモードに移行している時は、ペアリングモード標識を、BLE U2Fサービスに追加するようにします。

### ペアリングモードの判定

まずは、main関数の冒頭で、ペアリングモードに移行しているかどうかの判定をおこないます。<br>
`ble_stack_init` ---> `peer_manager_init` ---> `ble_u2f_pairing_get_mode` の順番で呼び出されるようにします。<br>
（後述しますが、ペアリングモード判定時にFDSを使用するための措置）

```
int main(void) {
  :
  // initialize ble stack.
  ble_stack_init();
  :
  // initialize peer manager.
  peer_manager_init();
  :
  // ペアリングモードをFDSから取得
  // (FDS初期化はpeer_manager_initの中で行われる)
  ble_u2f_pairing_get_mode(&m_u2f);
  :
```

#### ご参考：ペアリングモード判定時のFDSアクセスについて

MAIN SW長押しによりペアリングモードに移行させる場合は、FDS（Flash ROM）にペアリングモードデータを書き込んでからソフトデバイスを再起動させるようにしています。<br>
再起動完了時、FDSからペアリングモードデータがあれば、ペアリングモードに移行中であると判定するロジックとなっております。

ペアリング移行中は、BLEサービス開始前に、ディスカバリーを不可能とする設定を行う必要がありますが、この設定はソフトデバイス起動時でしか行うことができません（サービス開始後に設定変更はできません）。<br>
ディスカバリー可能／不可能の設定を切り替えるためには、ソフトデバイスの再起動が必要となってしまうため、前述のようなソフトデバイス再起動＋FDSアクセスを伴う実装としております。

ペアリングモード移行中は、`ble_u2f_pairing.c`内のモジュール変数`run_as_pairing_mode`の値が`true`に変化しています。<br>
これは`ble_u2f_pairing_mode_get`関数により、外部からアクセスできます。

```
// ペアリングモードを保持
static bool run_as_pairing_mode;

bool ble_u2f_pairing_mode_get(void) {
    return run_as_pairing_mode;
}
```

### キャラクタリスティックの設定

`ble_u2f_pairing_get_mode`の後に実行される`ble_u2f_init_services`の中で、ペアリングモード標識の設定を追加します。<br>
ペアリング標識モードの設定追加は、新設関数`u2f_pairing_mode_char_add `を実行して行います。
```
int main(void) {
    :
    // initialize ble services.
    :
    ble_services_init();
    :

static void ble_services_init(void) {
    :
    // Initialize FIDO U2F Service.
    err_code = ble_u2f_init_services(&m_u2f);
    :

uint32_t ble_u2f_init_services(ble_u2f_t * p_u2f) {
    :
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_u2f->service_handle);
    :
    err_code = u2f_pairing_mode_char_add(p_u2f);
    :

```

### キャラクタリスティックをサービスに追加

関数`u2f_pairing_mode_char_add`により、U2Fサービスにペアリングモード標識を追加します。<br>
ペアリングモード標識の仕様は以下になります。

- 長さ１バイトの属性
- 読取専用
- 更新通知は不要
- UUIDは先述のオリジナルUUID`98439EE6-776B-401C-880C-682FBDDD8E32`

FIDOのUUIDと同様、オリジナルUUIDは以下のように定義しておきます。

```
// Original UUID (98439EE6-776B-401C-880C-682FBDDD8E32)
#define BLE_UUID_U2F_PAIRING_MODE_CHAR 0x9EE6
static ble_uuid128_t original_base_uuid = {
    0x32, 0x8E, 0xDD, 0xBD, 0x2F, 0x68, 0x0C, 0x88, 0x1C, 0x40, 0x6B, 0x77, 0x00, 0x00, 0x43, 0x98
};
```

関数`u2f_pairing_mode_char_add`では、ペアリングモード標識にオリジナルUUIDを設定し、U2Fサービス内に追加しています。

```
static uint32_t u2f_pairing_mode_char_add(ble_u2f_t *p_u2f) {
    :
    //
    // オリジナルUUID(128bit)を採用
    //
    uint8_t  uuid_type_;
    uint32_t err_code = sd_ble_uuid_vs_add(&original_base_uuid, &uuid_type_);
    VERIFY_SUCCESS(err_code);
    ble_uuid.type = uuid_type_;
    ble_uuid.uuid = BLE_UUID_U2F_PAIRING_MODE_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));
    :
    attr_char_value.p_uuid    = &ble_uuid;
    :
    // U2Fサービス内に追加
    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_pairing_mode_handles);
}
```

キャラクタリスティック追加時に取得したハンドルは、接続共有情報`ble_u2f_t`に追加したフィールド`u2f_pairing_mode_handles`で保持されます。

```
struct ble_u2f_s
{
    :
    ble_gatts_char_handles_t u2f_status_handles;
    ble_gatts_char_handles_t u2f_control_point_handles;
    ble_gatts_char_handles_t u2f_control_point_length_handles;
    ble_gatts_char_handles_t u2f_service_revision_bitfield_handles;
    ble_gatts_char_handles_t u2f_service_revision_handles;
    ble_gatts_char_handles_t u2f_pairing_mode_handles;
    :
};

typedef struct ble_u2f_s ble_u2f_t;
```

### キャラクタリスティックの制御

ペアリングモードに移行中は、ペアリングモード標識を見せるようにする一方、U2F関連のキャラクタリスティックは見せないよう、関数`ble_u2f_init_services`内で制御します。

```
uint32_t ble_u2f_init_services(ble_u2f_t * p_u2f) {
    :
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_u2f->service_handle);
    :
    if (ble_u2f_pairing_mode_get()) {
        // ペアリングモードの場合はペアリングモード標識を追加
        err_code = u2f_control_point_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

    } else {
        // 非ペアリングモードの場合はU2F関連キャラクタリスティックを追加
        err_code = u2f_control_point_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

        err_code = u2f_status_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

        err_code = u2f_control_point_length_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

        err_code = u2f_service_revision_bitfield_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

        err_code = u2f_service_revision_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);
    }
    :
```
