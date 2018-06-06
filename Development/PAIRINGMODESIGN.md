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

## nRF52側の改修案

ペアリングモードに移行している時は、ペアリングモード標識を、BLE U2Fサービスに追加するようにします。

### キャラクタリスティック設定の追加

下記の流れで実行される`ble_u2f_init_services`の中で、ペアリングモード標識の設定を追加します。<br>
具体的には関数`u2f_pairing_mode_char_add `を新設します。
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

関数`u2f_pairing_mode_char_add`の内容は以下になります。

```
static uint32_t u2f_pairing_mode_char_add(ble_u2f_t *p_u2f)
{
    // 'OneCardPairingMode' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    //
    // オリジナルUUID(128bit)を採用
    //
    uint8_t  uuid_type_;
    uint32_t err_code = sd_ble_uuid_vs_add(&original_base_uuid, &uuid_type_);
    VERIFY_SUCCESS(err_code);
    ble_uuid.type = uuid_type_;
    ble_uuid.uuid = BLE_UUID_U2F_PAIRING_MODE_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 1;
    attr_char_value.p_value   = 0x00;

    // U2Fサービス内に追加
    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_pairing_mode_handles);
}
```

接続共有情報`ble_u2f_t`に`u2f_pairing_mode_handles`を追加し、キャラクタリスティック追加時に取得したハンドルを保持します。

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

ペアリングモードに移行時は、ペアリングモード標識を見せるようにする一方、U2F関連のキャラクタリスティックは見せないようにします。<br>
そのための制御は、関数`ble_u2f_init_services`内で実行する必要があります。


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

ペアリングモードは`ble_u2f_pairing.c`内のモジュール変数`run_as_pairing_mode`で保持しているので、これを外部からアクセスできるようにします。

```
// ペアリングモードを保持
static bool run_as_pairing_mode;

bool ble_u2f_pairing_mode_get(void) {
    return run_as_pairing_mode;
}
```
