# nRF5340アプリケーションについて

最終更新日：2022/4/27

## 概要

nRF5340アプリケーションに関する技術情報を掲載しています。

## ハードウェア依存コード

nRF5340やZephyrプラットフォームに依存する処理は、以下のコードに記述されています。

|モジュール名|内容|詳細|
|:--|:-|:-|
|`app_ble_fido`|BLE FIDOサービス関連|FIDO関連のBLE送受信処理|
|`app_ble_pairing`|BLEペアリング関連|ペアリングモードを保持<br>ペアリング完了／失敗時の処理<br>ペアリング情報削除処理|
|`app_ble_smp`|BLE DFU関連|BLE SMPサービスの各種設定処理|
|`app_bluetooth`|BLE関連メイン|BLEアドバタイズ開始／停止<br>BLEサービス開始<br>Zephyrプラットフォームからのコールバック|
|`app_board`|nRF5340基板関連|共通処理<br>ボタン関連<br>LED関連<br>システム再始動<br>ブートローダーモード遷移[注1]|
|`app_crypto_ec`|暗号化関連（EC）|ECDSA署名処理<br>EC鍵ペア生成<br>ECDH共通鍵生成|
|`app_crypto_rsa`|暗号化関連（RSA）|定数E管理<br>RSA-2048関連処理<br>（暗号化・公開鍵生成・鍵インポート）|
|`app_crypto`|暗号化関連メイン|CTR-DRBG共有情報<br>AES-256暗号化処理<br>ランダムベクター生成<br>SHA256ハッシュ生成<br>HMAC SHA256ハッシュ生成<br>3DES-ECB暗号化処理<br>初期設定|
|`app_event`|イベント関連|イベント待ち行列の管理<br>スレッドのイベント処理<br>データ関連イベント処理<br>スレッド本体の定義・生成|
|`app_flash`|Flash ROM関連|Flash ROM領域の空き容量取得|
|`app_main`|業務処理メイン|アプリケーション初期化処理<br>データ処理イベント関連<br>ボタン押下時の処理|
|`app_platform`|nRF52840アプリケーションからの<br>移行用関数群|トランスポート関連<br>DFU関連<br>管理コマンド用関数群<br>BLE自動認証関連(Dummy)|
|`app_process`|アプリケーション処理制御|ボタンイベント振分け<br>データチャネル関連処理<br>データ処理イベント振り分け|
|`app_settings`|データ永続化関連|永続化機能の初期化<br>データ検索／追加／削除<br>Zephyrプラットフォームからのコールバック|
|`app_status_indicator`|LED点滅制御関連|USB状態表示<br>LED点滅制御|
|`app_timer`|タイマー関連|ボタン長押し検知タイマー<br>アイドル状態連続検知タイマー<br>LED点滅用繰り返しタイマー<br>汎用ワンショットタイマー<br>汎用リピートタイマー|
|`app_usb_ccid`|USB CCIDインターフェース関連|CCID I/Fデスクリプター<br>CCID I/F送信処理／コールバック|
|`app_usb_hid`|USB HIDインターフェース関連|HID I/Fディスクリプター<br>USB HID I/F初期化処理<br>USB HID I/F送信処理／コールバック|
|`app_usb`|USBデバイス関連|USBデバイスのステータスを管理<br>USBデバイス初期処理<br>USBデバイス停止処理|
|`main`|ベースとなったBLE DISサンプル|コードはほぼ全てコメントアウト|

以下は主として、nRF52840での実装を、同じ関数名・引数を用いてnRF5340用に移植したモジュールです。<br>
一部、ラッピング関数を含んでいます。

|モジュール名|内容|
|:--|:-|
|`ccid_crypto`|PIV／OpenPGP機能で使用されるRSA関連処理|
|`ccid_flash_object`|PIV／OpenPGP機能で共通使用される永続化関連処理|
|`ccid_flash_openpgp_object`|OpenPGP機能で使用される永続化関連処理|
|`ccid_flash_piv_object`|PIV機能で使用される永続化関連処理|
|`fido_crypto`|FIDO機能で使用される暗号化／署名関連処理|
|`fido_flash`|FIDO機能で使用される永続化関連処理|
|`fido_timer`|FIDO機能で使用されるタイマー関連処理|

[注1] USB DFUで使用する予定でしたが、本プロジェクトでは採用を見送ったので、最終更新日現在、ブートローダーモード遷移機能を実行することは出来ません。

## ハードウェア非依存コード

[nRF52840アプリケーション](../nRF52840_app)と共用しているコードです。

|モジュール名|内容|詳細|
|:--|:-|:-|
|`CCID/ccid_lib`|CCID共通／PIV関連|CCIDトランスポート関連処理<br>各種認証処理<br>永続化関連処理<br>管理機能処理|
|`CCID/openpgp_lib`|OpenPGP関連|暗号化／署名関連処理<br>永続化関連処理<br>管理機能処理|
|`FIDO2Device/ctap2_lib`|CTAP2関連|CBOR生成／解析処理<br>PIN管理機能処理<br>WebAuthn認証処理|
|`FIDO2Device/fido2_lib`|FIDO2関連|HID／BLEトランスポート関連処理<br>暗号化／署名関連処理<br>永続化関連処理<br>管理機能処理<br>その他共通処理|
|`FIDO2Device/u2f_lib`|U2F関連|U2F認証処理<br>キーハンドル関連<br>ECDSA署名処理|

## 各種定義体

nRF5340やZephyrプラットフォームについての各種定義は下記の通りです。<br>
いずれも、最終更新日現在の設定値になります。

### nRF5340に関する定義

この定義は、nRF5340を使用する基板のデザイン（回路・配線等）で変更される場合があります。<br>
現状は、nRF5340 DKと同様の基板デザインであることを前提した定義となっています。

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_BT_DIS_FW_REV`|`y`||
|`CONFIG_BT_DIS_HW_REV`|`y`||
|`CONFIG_BT_DIS_FW_REV_STR`|`"0.4.6"`|ファームウェアのバージョンは、このエントリーで管理します。|
|`CONFIG_BT_DIS_HW_REV_STR`|`"PCA10095_01"`|基板名[注1]|
|`CONFIG_BT_DEVICE_NAME`|`"Secure device 53"`|デバイス名|
|`CONFIG_HEAP_MEM_POOL_SIZE`|`4096`||

[注1] `PCA10095`＝nRF5340 DK、`PCA10095_01`＝BT40 Slim Board

### nRF5340アプリケーションに関する定義

#### Bluetooth関連

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_BT`|`y`||
|`CONFIG_BT_PERIPHERAL`|`y`||
|`CONFIG_BT_GATT_DYNAMIC_DB`|`y`||
|`CONFIG_BT_LIM_ADV_TIMEOUT`|`90`|アドバタイズ停止までのタイムアウト秒数|
|`CONFIG_BT_L2CAP_TX_MTU`|`126`||
|`CONFIG_BT_BUF_ACL_RX_SIZE`|`128`||
|`CONFIG_BT_MAX_CONN`|`1`|最大接続数を`1`とし、複数端末／サービスからの<br>接続を許容しません。|
|`CONFIG_BT_MAX_PAIRED`|`10`|`10`端末までペアリングできます。|
|`CONFIG_BT_SETTINGS`|`y`||
|`CONFIG_BT_SMP`|`y`||
|`CONFIG_BT_SMP_SC_ONLY`|`y`|ペアリング時に６桁のパスコード入力を要求され<br>ます。[注2]|
|`CONFIG_BT_FIXED_PASSKEY`|`y`|固定のパスコードを使用します。[注2]|
|`CONFIG_BT_TINYCRYPT_ECC`|`y`||

[注1] ペアリングモードに遷移後、`90`秒以上経過した場合、このタイムアウト設定によりアドバタイズが停止するため、nRF5340アプリケーションでは、自動的に非ペアリングモードに遷移させるようにしています。<br>
[注2] パスコードは、ハードウェアIDから生成されます。詳細については[技術情報補足](../nRF5340_app/TECHNICAL.md)ご参照

#### デバイス情報関連

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_BT_DIS`|`y`||
|`CONFIG_BT_DIS_SETTINGS`|`y`||
|`CONFIG_BT_DIS_STR_MAX`|`21`||
|`CONFIG_BT_DIS_PNP`|`n`||
|`CONFIG_BT_DIS_MODEL`|`"Secure device"`||
|`CONFIG_BT_DIS_MANUF`|`"Diverta Inc."`||
|`CONFIG_BT_DIS_SERIAL_NUMBER`|`n`||
|`CONFIG_BT_DIS_FW_REV`|`n`|この設定は上書きされます。|
|`CONFIG_BT_DIS_HW_REV`|`n`|この設定は上書きされます。|
|`CONFIG_BT_DIS_SW_REV`|`n`||
|`CONFIG_BT_DIS_SERIAL_NUMBER_STR`|`""`||
|`CONFIG_BT_DIS_FW_REV_STR`|`""`|この設定は上書きされます。|
|`CONFIG_BT_DIS_HW_REV_STR`|`""`|この設定は上書きされます。|
|`CONFIG_BT_DIS_SW_REV_STR`|`""`||
|`CONFIG_BT_DEVICE_NAME`|`"Secure device"`|この設定は上書きされます。|

#### USB関連

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_USB_DEVICE_STACK`|`y`||
|`CONFIG_USB_DEVICE_HID`|`y`||
|`CONFIG_USB_DEVICE_VID`|`0xf055`|仮の設定 [注1]|
|`CONFIG_USB_DEVICE_PID`|`0x0001`|仮の設定 [注1]|
|`CONFIG_USB_DEVICE_MANUFACTURER`|`"Diverta Inc."`||
|`CONFIG_USB_DEVICE_PRODUCT`|`"Secure Dongle"`||
|`CONFIG_USB_DEVICE_SN`|`"000000000000"`|変更不可 [注1]|
|`CONFIG_USB_DRIVER_LOG_LEVEL_OFF`|`y`||
|`CONFIG_USB_DEVICE_LOG_LEVEL_OFF`|`y`||
|`CONFIG_ENABLE_HID_INT_OUT_EP`|`y`||
|`CONFIG_HID_INTERRUPT_EP_MPS`|`64`||
|`CONFIG_USB_HID_POLL_INTERVAL_MS`|`10`||
|`CONFIG_USB_HID_BOOT_PROTOCOL`|`n`||
|`CONFIG_USB_HID_LOG_LEVEL_OFF`|`y`||

[注1] [USB-IF](https://www.usb.org)でいまだ割り当てられていないとされるベンダーIDですが、製品化の際は割り当てを検討する必要があります。<br>
[注2] Zephyrでは、USBデバイスのシリアル番号を、ハードウェアIDから自動的に取得して生成しています。詳細については[技術情報補足](../nRF5340_app/TECHNICAL.md)ご参照

#### 永続化関連

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_FLASH`|`y`||
|`CONFIG_FLASH_MAP`|`y`||
|`CONFIG_FLASH_PAGE_LAYOUT`|`y`||
|`CONFIG_FLASH_LOG_LEVEL_OFF`|`y`||
|`CONFIG_NVS`|`y`||
|`CONFIG_NVS_LOG_LEVEL_OFF`|`y`||
|`CONFIG_SETTINGS`|`y`||
|`CONFIG_SETTINGS_LOG_LEVEL_OFF`|`y`||
|`CONFIG_PM_PARTITION_SIZE_SETTINGS_STORAGE`|`0x4000`|永続化領域全体のバイト数（＝`16,384bytes`）<br>このサイズを超えた場合、データをFlash ROMに<br>保存できなくなります。|
|`CONFIG_APP_SETTINGS_BUFFER_SIZE`|`1024`|永続化項目１件あたりの最大バイト数。<br>これを超える長さのデータは、Flash ROMに保存<br>できません。|

#### 暗号化関連

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_NORDIC_SECURITY_BACKEND`|`y`||
|`CONFIG_NRF_SECURITY_RNG`|`y`||
|`CONFIG_ENTROPY_GENERATOR`|`y`||
|`CONFIG_MBEDTLS`|`y`||
|`CONFIG_MBEDTLS_LIBRARY`|`y`||
|`CONFIG_MBEDTLS_ENABLE_HEAP`|`y`||
|`CONFIG_MBEDTLS_HEAP_SIZE`|`65536`||
|`CONFIG_MBEDTLS_INSTALL_PATH`|`"DUMMY"`||
|`CONFIG_MBEDTLS_CTR_DRBG_C`|`y`||
|`CONFIG_MBEDTLS_SHA384_C`|`n`||
|`CONFIG_MBEDTLS_SHA512_C`|`n`||
|`CONFIG_MBEDTLS_CIPHER_MODE_XTS`|`n`||
|`CONFIG_MBEDTLS_CHACHA20_C`|`n`||
|`CONFIG_MBEDTLS_POLY1305_C`|`n`||
|`CONFIG_MBEDTLS_CHACHAPOLY_C`|`n`||
|`CONFIG_MBEDTLS_GCM_C`|`n`||

#### その他
|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_GPIO`|`y`||
|`CONFIG_PM`|`y`||
|`CONFIG_PM_DEVICE`|`y`||
|`CONFIG_COUNTER`|`y`||
|`CONFIG_COUNTER_RTC0`|`y`||
|`CONFIG_COMPILER_OPT`|`"-DFIDO_ZEPHYR -DMBEDTLS_DES_C"`|特別なコンパイラーオプションは、<br>ここで追加指定します。|
|`CONFIG_BT_DEBUG_LOG`|`n`||
|`CONFIG_BT_DEBUG_NONE`|`n`||
|`CONFIG_BT_DEBUG_SMP`|`n`||
|`CONFIG_RESET_ON_FATAL_ERROR`|`n`||

### BLE DFUに関する定義

#### 基本設定
サンプル`smp_svr/prj.conf`から転記した内容です（一部変更あり）。

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_MCUMGR`|`y`||
|`CONFIG_BOOTLOADER_MCUBOOT`|`y`||
|`CONFIG_THREAD_MONITOR`|`y`||
|`CONFIG_STATS`|`n`|サンプル（`y`）から修正|
|`CONFIG_STATS_NAMES`|`n`|サンプル（`y`）から修正|
|`CONFIG_MCUMGR_CMD_IMG_MGMT`|`y`||
|`CONFIG_MCUMGR_CMD_OS_MGMT`|`y`||
|`CONFIG_MCUMGR_CMD_STAT_MGMT`|`n`|サンプル（`y`）から修正|
|`CONFIG_LOG`|`y`||
|`CONFIG_MCUBOOT_UTIL_LOG_LEVEL_WRN`|`y`||

#### Bluetooth関連設定
サンプル`smp_svr/overlay-bt.conf`から転記した内容です（一部変更あり）。

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_BT_L2CAP_TX_MTU`|`252`||
|`CONFIG_BT_BUF_ACL_RX_SIZE`|`256`||
|`CONFIG_MCUMGR_SMP_BT`|`y`||
|`CONFIG_MCUMGR_SMP_BT_AUTHEN`|`y`|転送内容は暗号化されます。<br>サンプル（`n`）から修正|
|`CONFIG_MCUMGR_SMP_BT_CONN_PARAM_CONTROL`|`y`||
|`CONFIG_MCUMGR_SMP_SHELL`|`n`|サンプル（`y`）から修正|
|`CONFIG_FILE_SYSTEM`|`n`|サンプル（`y`）から修正|
|`CONFIG_FILE_SYSTEM_LITTLEFS`|`n`|サンプル（`y`）から修正|
|`CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE`|`2304`||
|`CONFIG_MCUMGR_CMD_FS_MGMT`|`n`|サンプル（`y`）から修正|
