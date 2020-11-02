# BLEデバイスによる自動認証機能について

## 概要
FIDO認証（WebAuthn／U2F）実行時、MDBT50Q Dongle上のボタンを押す代わりに、One CardなどのBLEデバイスを近づけることにより、認証処理を自動的に続行させる機能です。

## 前提

MDBT50Q Dongleが、PCのUSBポートに装着されていることが前提となります。<br>
すなわち、PC側に対してはUSB HIDデバイスとして機能し、本機能のために使用するBLEデバイスに対してはBLEセントラルデバイスとして機能することになります。

## 操作方法
「<b>[BLEデバイスによる自動認証機能](BLEDAUTH.md)</b>」をご参照願います。

## プログラムの実装

プログラム実装に関する情報を掲載いたします。

- パラメーター設定時の処理<br>
本機能を利用するために必要なパラメーター（自動認証パラメーター）を、MDBT50Q Dongle側に事前設定します。

- ユーザー登録時の処理<br>
ユーザー登録処理（U2F Register／CTAP2 authenticatorMakeCredential）時には、自動認証パラメーターの内容に応じて、BLEデバイススキャンまたはボタン押下を待機します。

- ログイン時の処理<br>
ログイン処理（U2F Authenticate／CTAP2 authenticatorGetAssertion）時には、ユーザー登録時のBLEスキャンパラメーターの内容に応じて、BLEデバイススキャンまたはボタン押下を待機します。

#### パラメーター設定時の処理

[管理ツール](../../MaintenanceTool/README.md)の「ツール設定」画面から、MDBT50Q Dongleに対して、後述「自動認証関連コマンド」が実行されます。

```
static void command_preference_parameter_maintenance(void) {
    :
    switch (cmd_type) {
        case 1:
        case 2:
        case 3:
            demo_ble_peripheral_auth_param_request(data, length);
            ret = demo_ble_peripheral_auth_param_response(cmd_type, buffer, &buffer_size);
            break;
            :
```

`demo_ble_peripheral_auth_param_request` では、管理ツールから受領したCSVから、パラメーター項目を取り出します。<br>
取り出したパラメーター項目は、後述「自動認証設定ファイル」(Flash ROM内)に保持されます。

```
void demo_ble_peripheral_auth_param_request(uint8_t *request, size_t request_size) {
    :
    // データの１バイト目からコマンド種別を取得
    switch (request[0]) {
        :
        case 2:
            // 書出の場合
            // CSVを各項目に分解し、内部変数に設定
            parse_auth_param_request(request, request_size);
            // パラメーターをFlash ROMに保存
            save_auth_param();
            break;
            :
```

#### ユーザー登録時の処理

ユーザー登録処理（U2F Register／CTAP2 authenticatorMakeCredential）時には、自動認証パラメーターをFlash ROMの自動認証設定ファイルから読み出します（関数`demo_ble_peripheral_auth_param_init`を実行）。


自動認証パラメーターの「自動認証フラグ」が有効化（＝BLEデバイスによる自動認証が有効化）されている場合は、BLEデバイススキャンにより、ユーザー所在確認を実行します。

```
static bool u2f_command_register_is_tup_needed(void) {
    // BLEデバイスによる自動認証が有効化されている場合は、
    // リクエストパラメーターの内容に関係なく、
    // ユーザー所在確認の代わりとなる
    // BLEデバイススキャンが行われるようにする
    // （管理ツールの設定画面で設定したサービスUUIDを使用）
    demo_ble_peripheral_auth_param_init();
    if (demo_ble_peripheral_auth_scan_enable()) {
        return true;
    }
    :
```

自動認証フラグ有効化の判定は、関数`demo_ble_peripheral_auth_scan_enable`で行なっています。

```
#define SCAN_ENABLE_DEFAULT 0
:
bool demo_ble_peripheral_auth_scan_enable(void) {
    :
    if (service_uuid_string[0] == 0 ||
        service_uuid_scan_enable == SCAN_ENABLE_DEFAULT) {
        // スキャン対象サービスUUIDが指定されていない場合、
        // または自動認証有効化フラグが設定されていない場合は、
        // 利用不可なので、falseを戻す
        return false;
    }

    return true;
}
```

BLEデバイスによる自動認証が有効化されている場合は、のちに行われるログイン処理時、BLEデバイススキャンの要否／スキャン対象UUID・アドレスといったパラメーター（BLEスキャンパラメーター）を生成し、「U2Fキーハンドル」または「CTAP2クレデンシャルID」に内包させるようにします。<br>
(U2Fキーハンドル、CTAP2クレデンシャルIDの内容は暗号化されます）

<b>【U2Fキーハンドル生成】</b>
```
void u2f_keyhandle_generate(uint8_t *p_appid_hash) {
    :
    // BLE自動認証機能用のスキャンパラメーターを末尾に追加
    //  先頭バイト: パラメーター長
    //  後続バイト: パラメーターのバイト配列を格納
    offset += demo_ble_peripheral_auth_scan_param_prepare(keyhandle_base_buffer + offset);
    :
    // AES暗号化対象のバイト配列＆長さを指定し、暗号化を実行
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    fido_command_aes_cbc_encrypt(keyhandle_base_buffer, keyhandle_buffer_block_size, keyhandle_buffer);
}
```

<b>【CTAP2クレデンシャルID生成】</b>
```
uint8_t ctap2_make_credential_generate_response_items(void) {
    :
    // Public Key Credential Sourceを編集する
    ctap2_pubkey_credential_generate_source(
        &ctap2_request.cred_param, &ctap2_request.user);

    // credentialIdを生成
    ctap2_pubkey_credential_generate_id();
    :
}

void ctap2_pubkey_credential_generate_source(CTAP_PUBKEY_CRED_PARAM_T *param, CTAP_USER_ENTITY_T *user) {
    // Public Key Credential Sourceを編集する
    :
    // BLE自動認証機能用のスキャンパラメーターを末尾に追加
    //  先頭バイト: パラメーター長
    //  後続バイト: パラメーターのバイト配列を格納
    offset += demo_ble_peripheral_auth_scan_param_prepare(pubkey_cred_source + offset);
    :
}

void ctap2_pubkey_credential_generate_id(void) {
    // Public Key Credential Sourceを
    // AES CBCで暗号化し、
    // credentialIdを生成する
    memset(credential_id, 0x00, sizeof(credential_id));
    fido_command_aes_cbc_encrypt(pubkey_cred_source, pubkey_cred_source_block_size, credential_id);
    credential_id_size = pubkey_cred_source_block_size;
    :
}
```

「U2Fキーハンドル」または「CTAP2クレデンシャルID」は、ユーザー登録処理時のレスポンスとして、WebAuthnクライアント（Webブラウザー）に引き渡されます。

#### ログイン時の処理

ログイン処理（U2F Authenticate／CTAP2 authenticatorGetAssertion）時には、前述の自動認証パラメーターではなく、ユーザー登録時のBLEスキャンパラメーターの内容に応じて、BLEデバイススキャンまたはボタン押下を待機します。

ログイン処理時に、WebAuthnクライアントから受け取ったリクエストデータには、先述の「U2Fキーハンドル」または「CTAP2クレデンシャルID」が同梱されています。<br>
MDBT50Q Dongleは、その中に内包されているBLEスキャンパラメーターを抽出します。

<b>【U2Fキーハンドルからの抽出】</b>
```
void u2f_keyhandle_restore(uint8_t *keyhandle_value, uint32_t keyhandle_length) {
    :
    // AES暗号化されたバイト配列を復号化
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    fido_command_aes_cbc_decrypt(keyhandle_buffer, keyhandle_length, keyhandle_base_buffer);
}

uint8_t *u2f_keyhandle_ble_auth_scan_param(void) {
    // BLEスキャンパラメーター格納領域の先頭を戻す
    return (keyhandle_base_buffer + U2F_APPID_SIZE + U2F_PRIVKEY_SIZE);
}
```

<b>【CTAP2クレデンシャルIDからの抽出】</b>
```
static void ctap2_pubkey_credential_restore_source(uint8_t *credential_id, size_t credential_id_size) {
    // authenticatorGetAssertionリクエストから取得した
    // credentialIdを復号化
    memset(pubkey_cred_source, 0, sizeof(pubkey_cred_source));
    fido_command_aes_cbc_decrypt(credential_id, credential_id_size, pubkey_cred_source);
    :
}

uint8_t *ctap2_pubkey_credential_ble_auth_scan_param(void) {
    :
    // BLEスキャンパラメーター格納領域の開始インデックスを取得
    offset = offset + 1 + src_user_id_size + 32;

    // BLEスキャンパラメーター格納領域の先頭を戻す
    return (pubkey_cred_source + offset);
}
```

抽出されたBLEスキャンパラメーターを、専用関数を使用して、ユーザー所在確認処理（関数`fido_user_presence_verify_start`）に引き渡します。

<b>【U2F】</b>
```
static void u2f_command_authenticate(void) {
    :
    if (u2f_command_authenticate_is_tup_needed()) {
      // ユーザー所在確認が必要な場合は、ここで終了し
      // その旨のフラグを設定
        is_tup_needed = true;
        fido_log_info("U2F Authenticate: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_user_presence_verify_start(U2F_KEEPALIVE_INTERVAL_MSEC,
            u2f_keyhandle_ble_auth_scan_param());
        return;
    }
    :
}
```

<b>【CTAP2】</b>
```
static void command_authenticator_get_assertion(void) {
    :
    if (ctap2_get_assertion_is_tup_needed()) {
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        // キープアライブ送信を開始
        fido_log_info("authenticatorGetAssertion: waiting to complete the test of user presence");
        fido_user_presence_verify_start(CTAP2_KEEPALIVE_INTERVAL_MSEC,
            ctap2_pubkey_credential_ble_auth_scan_param());
        return;
    }
    :
}

```

MDBT50Q Dongleは、抽出されたBLEスキャンパラメーターの内容に応じ、BLEデバイススキャンを実行するのか、ボタン押下を待機するのかを判断することになります。<br>
（BLEスキャンパラメーターは、`fido_user_presence_verify_start`の引数`context`に引き渡されます）

```
void fido_user_presence_verify_start(uint32_t timeout_msec, void *context) {
    :
    // 自動認証機能が有効な場合は
    // ボタンを押す代わりに
    // 指定のサービスUUIDをもつBLEペリフェラルをスキャン
    if (demo_ble_peripheral_auth_start_scan(context)) {
        return;
    }

    // LED点滅を開始し、ボタンの押下を待つ
    fido_status_indicator_prompt_tup();
}
```

#### BLEデバイスのスキャン

先述`demo_ble_peripheral_auth_start_scan`の呼び出しにより、BLEデバイススキャンが始まります。<br>
BLEデバイスのスキャンは、管理ツールの設定画面で設定されたスキャン秒数（`service_uuid_scan_sec`）の間行われます。

```
bool demo_ble_peripheral_auth_start_scan(void *context) {
    if (context != NULL) {
        // ログイン処理の場合は、
        // BLEスキャンパラメーターを引き渡す
        return demo_ble_peripheral_auth_start_second_scan((uint8_t *)context);
    }
    ：
    // 指定したサービスUUIDを使用し、
    // 指定秒数間スキャンを実行
    ble_service_central_scan_start(service_uuid_scan_sec * 1000, resume_function_after_scan, true);
    return true;
}

static bool demo_ble_peripheral_auth_start_second_scan(uint8_t *p_scan_param)
{
    ：
    // 指定したサービスUUIDを使用し、指定秒数間スキャンを実行
    ble_service_central_scan_start(service_uuid_scan_sec * 1000, resume_function_after_scan, false);
    return true;
}
```

スキャンが完了すると、`resume_function_after_scan`という関数が呼び出されます。<br>
ここでは、スキャンされたBLEデバイスが、管理ツールの設定画面で設定されたスキャン対象サービスUUIDと合致しているかどうかのチェックが行われます。[注1]

BLEデバイスとしてスマートフォンが使用される（＝ペアリング要否フラグ`service_uuid_need_pairing`が設定されている）場合は、実際にスマートフォンに対しセキュア接続を要求し、正式なBluetoothアドレスの取得を試行します。[注2][注3]

BLEデバイスとしてスマートフォンが使用されない場合は、ここでユーザー所在確認が行われたと判断し、後続の処理（署名処理、署名カウンター更新、等）が実行されます。

```
static void resume_function_after_scan(bool is_register) {
  ：
  // スキャン対象サービスUUIDが、スキャン統計情報に含まれているかどうかチェック
  ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(service_uuid_string);
  if (info == NULL) {
      // 見つからなかった時の処理
      ：
  } else {
      // 見つかった時の処理
      // 複数スキャンされた場合は、最もRSSI値が大きいBLEデバイスが戻ります。
      ：
      // ペアリングが必要な場合は、セキュア接続を要求
      if (service_uuid_need_pairing != 0) {
          perform_secure_connection(is_register, info);
          return;
      }

      // 後続の処理を実行
      bool found = register_or_match_scan_param(is_register, info->uuid_bytes, info->uuid_bytes_size, info->peer_addr.addr);
      fido_user_presence_verify_on_ble_scan_end(found);
  }
```

[注1] 同じサービスUUIDを持つBLEデバイスが、複数スキャンされた場合は、最もRSSI値が大きい（＝最も近接していると考えられる）BLEデバイスが、スキャン結果として１件のみ選択されます。<br>
[注2] スマートフォンの場合は、スキャン統計情報に含まれるBluetoothアドレスが、ランダムな仮アドレスとなっています。この仮アドレスを使用した場合、ユーザー登録時とログイン時でBluetoothアドレスの値が異なり、合致チェックに失敗することになります。そのため、ユーザー登録時とログイン時で正式なBluetoothアドレスを取得するようにしたものです。<br>
[注3] 正式なBluetoothアドレスは、スマートフォン〜nRF52840間でペアリングが事前に行われており、かつ、セキュア接続が行われたときにしか取得できない仕様となっています。

#### 正式なBluetoothアドレスの取得

先述の専用関数`perform_secure_connection`の実行により、BLEデバイスとして使用するスマートフォンと、nRF52840間で、セキュア接続が試行されます。<br>
セキュア接続要求は、ソフトデバイスのAPI`sd_ble_gap_connect`を使用して行います。

```
static void perform_secure_connection(bool is_register, ADV_STAT_INFO_T *info) {
    // スキャンできたBLEデバイスに接続し、正式なBluetoothアドレスを取得
    if (ble_service_central_request_connection(&info->peer_addr, resume_after_secure_connection, is_register) == false) {
      ：
}

bool ble_service_central_request_connection(ble_gap_addr_t *p_addr, void (*_resume_function)(bool), bool _resume_func_after_conn_param) {
    ：
    // Establish connection.
    ret_code_t err_code = sd_ble_gap_connect(p_addr, p_scan_params, p_conn_params, con_cfg_tag);
    ：
    // 接続後に実行される関数の参照を退避
    resume_func_after_conn = _resume_function;
    resume_func_after_conn_param = _resume_func_after_conn_param;
    return true;
}
```

セキュア接続が確立されると、`resume_after_secure_connection`という関数が実行されます。[注1]<br>
ここで、正式なBluetoothアドレスを取得することが可能となります。

```
static void resume_after_secure_connection(bool is_register) {
    if (ble_service_central_already_paired()) {
        // ペアリング情報により接続できたら、
        // 正式なBluetoothアドレスを抽出
        uint8_t *uuid_bytes = adv_stat_info_scan_matched->uuid_bytes;
        size_t   uuid_bytes_size = adv_stat_info_scan_matched->uuid_bytes_size;
        uint8_t *connected_address = ble_service_central_connected_address();

        // 後続の処理を実行
        bool found = register_or_match_scan_param(is_register, uuid_bytes, uuid_bytes_size, connected_address);
        fido_user_presence_verify_on_ble_scan_end(found);
        ：
    }

    // BLE切断
    ble_service_central_request_disconnection();
}
```

正式アドレスが取得できたら、ユーザー登録時はそれをBLEスキャンパラメーターとして登録し、ログイン時はBLEスキャンパラメーターとのマッチングが行われます。<br>
結果として、ユーザー登録時とログイン時に、使用したスマートフォンが同一であることがチェックされることになります。

```
static bool register_or_match_scan_param(bool is_register, uint8_t *uuid_bytes, size_t uuid_bytes_size, uint8_t *connected_address) {
    bool found = true;
    if (is_register) {
        // Registerの場合は、BLEスキャンパラメーターを登録するため
        // 統計情報とBluetoothアドレスをバッファに保持
        scan_parameter_buffer_set(uuid_bytes, uuid_bytes_size, connected_address);

    } else {
        // Authenticateの場合は、BLEスキャンパラメーターに登録された
        // Bluetoothアドレスとマッチングを行う
        if (ble_service_central_stat_match_scan_param(scan_param_bytes, uuid_bytes, uuid_bytes_size, connected_address) == false) {
            // 失敗した場合は以降の処理を中止
            found = false;
        }
    }

    // マッチング結果を戻す
    ：
    return found;
}
```

[注1] 接続確立後は、別段のサービス／キャラクタリスティック・ディスカバリーは行われず、即切断となります。

## プログラムの仕様

プログラムの仕様に関する情報を掲載いたします。

#### 自動認証関連コマンド

[管理ツール](../../MaintenanceTool/README.md)の「ツール設定」画面から、MDBT50Q Dongleに対して実行されるコマンドです。

- 設定読込<br>
MDBT50Q Dongleで保持されている自動認証パラメーター（自動認証に関するパラメーター）を、CSVで読込みます。

- 設定書込<br>
管理ツール画面で設定／変更された自動認証パラメーターを、CSVでMDBT50Q Dongleに転送します。<br>
転送内容はMDBT50Q Dongle側で保持されます。

- 設定解除<br>
MDBT50Q Dongleで保持されている自動認証パラメーターをクリアします。

| # |項目 |リクエスト | レスポンス |
|:-:|:-|:-|:-|
|1|設定読込|`<チャネルID> c4 00 01 01`|`<チャネルID> c4 <データ長> <ステータス> <設定内容CSV>`|
|2|設定書込|`<チャネルID> c4 <データ長>`<br>`02 <設定内容CSV>`|`<チャネルID> c4 <データ長> <ステータス> <設定内容CSV>`|
|3|設定解除|`<チャネルID> c4 00 01 03`|`<チャネルID> c4 00 05 <ステータス> 30 2c 2c 33`|

[注1] チャネルID（4バイト）は`01 00 33 01`等のバイト配列になります。事前にCTAP2_INITコマンドを実行する必要があります。<br>
[注2] データ長（2バイト）は、設定内容CSVのサイズ＋1バイト（ステータス）になります。設定内容CSVのサイズが４バイトの場合、データ長は`00 05`となります。<br>
[注3] ステータス（1バイト）は、`00`は正常終了、それ以外の値は異常終了を意味します。<br>
[注4] 設定内容CSV（可変長）は、自動認証に関するパラメーターの内容をCSV化した値になります。レイアウトは`<自動認証フラグ>,<スキャン対象サービスUUID文字列>,<スキャン秒数>`となります。例えば`1,422E0000-E141-11E5-A837-0800200C9A66,3`といった値になります。

#### 自動認証設定ファイル

前述「自動認証関連コマンド」の実行により、管理ツールからMDBT50Q Dongleに引き渡された、自動認証パラメーターを保持します。<br>
nRF52840内のFlash ROMにより永続化されます。

|# |位置[注1] |項目名 |説明 |
|:-|:-:|:-|:-|
|1|0 - 8|スキャン対象サービス<br>UUID文字列|指定したUUIDのサービスを持つBLEデバイスが、スキャン対象となります。|
|2|9 |スキャン秒数|指定した秒数の間、BLEデバイスをスキャンします。|
|3|10 |自動認証フラグ|自動認証時の各種フラグ（自動認証有効化フラグ[注2]／ペアリング要否フラグ[注3]）を１ワードにまとめた値です。[注4]|

[注1] 位置はワード単位になります（1ワード＝4バイト）。<br>
[注2] ユーザー登録時に、BLEデバイスによる自動認証を有効化するかどうかのフラグです。1:有効化　0:無効化<br>
[注3] ペアリングの要否を判定するフラグです。1:必要（BLEデバイスとしてスマートフォンを使用する場合、ペアリングが必要）0:不要<br>
[注4] 最初の下位１バイト分が自動認証有効化フラグとなり、次の１バイト分がペアリング要否フラグとなります。したがって、自動認証フラグのバイトイメージとしては`<予備バイト><予備バイト><ペアリング要否フラグ><自動認証有効化フラグ>` となります。例えば`0x00000101`といった値になります。

#### BLEスキャンパラメーター

MDBT50Q Dongleは、ユーザー登録時に、下表のような「BLEスキャンパラメーター」を生成し、FIDO機能で使用する「U2Fキーハンドル」または「CTAP2クレデンシャルID」に含めて管理します。

|# |位置 |項目名 |説明 |
|:-|:-:|:-:|:-|
|1|0 |パラメーター長|ユーザー登録時にBLEデバイススキャンが行われた場合は、<br>項番2、3の長さ（22）が設定されます。<br>ボタン押下により登録された場合は、0が設定されます。|
|2|1 - 16 |サービスUUID|ユーザー登録時にスキャンされたBLEデバイスのUUID。<br>ボタン押下により登録された場合は、格納されません。|
|3|17 - 22 |Bluetoothアドレス|ユーザー登録時にスキャンされたBLEデバイスのアドレス。<br>ボタン押下により登録された場合は、格納されません。|

ログイン処理では、MDBT50Q Dongleは上記BLEスキャンパラメーターを参照し、ユーザー登録時に自動認証が使用されたか否かの判別をします。<br>
結果、ログイン時の動きとしては、以下のようになります。

|# |条件|動作 |
|:-|:-|:-|
|1|BLEスキャンパラメーター長=0|BLEデバイススキャンを行わず、ボタン押下待ちを行います。<br>ボタン押下を検知したら、ユーザー所在確認は成功となり、ログインができるようになります。<br>自動認証フラグの現在設定値は無視されます。|
|2|BLEスキャンパラメーター長>0|BLEデバイススキャンを行います。<br>自動認証パラメーターで指定したスキャン秒数の間スキャンします。<br>スキャンパラメーターのUUID／アドレスと同じデバイスがスキャンされたら、ユーザー所在確認は成功となり、ログインができるようになります。<br>結果として、ユーザー登録時と同じBLEデバイスだけが、ログイン可能となります。|
