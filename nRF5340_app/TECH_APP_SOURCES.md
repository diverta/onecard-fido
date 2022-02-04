# nRF5340アプリケーションについて

最終更新日：2022/02/04

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
|`app_dfu`|USB DFU関連[注2]|ブートローダーモード遷移処理<br>DFU変更内容コミット処理|
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

[注1] USB DFUで使用する予定でしたが、本プロジェクトでは採用を見送ったので、最終更新日現在、ブートローダーモード遷移機能を実行することは出来ません。<br>
[注2] USB DFUは、本プロジェクトでは採用を見送ったので、最終更新日現在、このモジュールは機能しておりません。

## 各種定義体

nRF5340やZephyrプラットフォームについての各種定義は下記の通りです。<br>
いずれも、最終更新日現在の設定値になります。

#### nRF5340に関する定義

この定義は、nRF5340を使用する基板のデザイン（回路・配線等）で変更される場合があります。<br>
現状は、nRF5340 DKと同様の基板デザインであることを前提した定義となっています。

|定義名|設定値|内容|
|:--|:-|:-|
|`CONFIG_BT_DIS_FW_REV`|`y`||
|`CONFIG_BT_DIS_HW_REV`|`y`||
|`CONFIG_BT_DIS_FW_REV_STR`|`"0.4.2"`|ファームウェアのバージョンは、このエントリーで管理します。|
|`CONFIG_BT_DIS_HW_REV_STR`|`"PCA10095"`|基板名|
|`CONFIG_BT_DEVICE_NAME`|`"Secure device 53"`|デバイス名|
|`CONFIG_HEAP_MEM_POOL_SIZE`|`4096`||
