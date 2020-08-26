# ATECC608A専用ライブラリー

## 概要
ATECC608AをnRF52840とI2C接続し、セキュアICとして利用するためのモジュールです。

## 目的・背景

#### cryptoauthlib のライセンス抵触回避

ATECC608Aの組込調査においては、Microchip Technology社のライブラリー「[cryptoauthlib](https://github.com/MicrochipTech/cryptoauthlib)」を使用しておりましたが、別件「[オープンソースコードライセンスについての調査](https://github.com/diverta/onecard-fido/pull/346)」で調査を進めた結果、[ライセンス](https://github.com/MicrochipTech/cryptoauthlib/blob/master/license.txt)に抵触する可能性が高いことが判明しています。

[ライセンス](https://github.com/MicrochipTech/cryptoauthlib/blob/master/license.txt)の原文より
```
(c) 2015-2018 Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software
and any derivatives exclusively with Microchip products. It is your
responsibility to comply with third party license terms applicable to your
use of third party software (including open source software) that may
accompany Microchip software.
```

このため、cryptoauthlibの実装内容を転載する（ファイルやコードのコピー等）のではなく、実装仕様を改めてプログラムから抽出／整理した上で、フルスクラッチビルドにより移植対応を行うものです。

## 構成

#### モジュール一覧

移植作業中に追加／削除される可能性があります。

|#|ファイル名|説明|
|:---:|:---|:---|
|1-1|atecc_aes.c/.h|ATECC608A組込みAES関連|
|1-2|atecc_command.c/.h|各種コマンド実行|
|1-3|atecc_device.c/.h|デバイス制御|
|1-4|atecc_iface.c/.h|nRF52840〜ATECC608A間のI/F|
|1-5|atecc_nonce.c/.h|NONCE生成関連|
|1-6|atecc_priv.c/.h|外部秘密鍵導入関連|
|1-7|atecc_read.c/.h|各種データ照会関連|
|1-8|atecc_setup.c/.h|各種データ照会関連|
|1-9|atecc_sign.c/.h|ECDSA署名関連|
|1-10|atecc_util.c/.h|各種ユーティリティー関連|
|1-11|atecc_write.c/.h|各種データ更新関連|
|1-12|atecc.c/.h|エントリーモジュール|

#### 関数一覧

移植作業中に追加／削除される可能性があります。

|#|モジュール名|関数名|説明|
|:---:|:---|:---|:---|
|2-1|atecc_aes|atecc_aes_set_persistent_latch|AES初期化|
|2-2|atecc_aes|atecc_aes_encrypt|AES暗号化|
|2-3|atecc_aes|atecc_aes_decrypt|AES復号化|
|2-4|atecc_nonce|atecc_calculate_nonce|NONCE計算|
|2-5|atecc_nonce|atecc_nonce_rand|NONCE値生成|
|2-6|atecc_nonce|atecc_nonce_load|NONCE値照会|
|2-7|atecc_priv|atecc_priv_write|所定のスロットに、外部秘密鍵を導入|
|2-8|atecc_read|atecc_read_zone|データ領域照会|
|2-9|atecc_read|atecc_read_bytes_zone|メモリー領域照会|
|2-10|atecc_read|atecc_read_config_zone|Config領域照会|
|2-11|atecc_read|atecc_read_serial_number|シリアル番号照会|
|2-12|atecc_setup|atecc_setup_config|Config情報の初回設定|
|2-13|atecc_sign|atecc_sign|所定スロットの秘密鍵を使用し、ECDSA署名を生成|
|2-14|atecc_sign|atecc_verify_extern|外部公開鍵を使用し、ECDSA署名を検証|
|2-15|atecc_util|atecc_get_address|メモリーアドレス照会|
|2-16|atecc_util|atecc_get_zone_size|メモリー領域(Zone)サイズ照会|
|2-17|atecc_util|atecc_lock_config_zone|Config領域ロック|
|2-18|atecc_util|atecc_lock_data_zone|データ領域ロック|
|2-19|atecc_util|atecc_lock_status_get|デバイスロック状態取得|
|2-20|atecc_util|atecc_random|ランダム値生成|
|2-21|atecc_util|atecc_gen_key|所定スロットの秘密鍵を使用し、公開鍵を生成|
|2-22|atecc_util|atecc_info_set_latch|AESを使用可能にするためのラッチ[注1]を設定|
|2-23|atecc_util|atecc_info_get_latch|AESを使用可能にするためのラッチ[注1]の状態照会|
|2-24|atecc_write|atecc_write_zone|データ領域更新|
|2-25|atecc_write|atecc_write_bytes_zone|メモリー領域更新|
|2-26|atecc_write|atecc_write_config_zone|Config領域更新|
|2-27|atecc|atecc_is_available|デバイス使用可否照会|
|2-28|atecc|atecc_initialize|デバイス初期化|
|2-29|atecc|atecc_finalize|デバイス解放|
|2-30|atecc|atecc_get_serial_num_str|シリアル番号文字列取得|
|2-31|atecc|atecc_get_config_bytes|デバイス設定データ取得|
|2-32|atecc|atecc_setup_config|デバイス初回設定処理|
|2-33|atecc|atecc_install_privkey|外部秘密鍵を導入|
|2-34|atecc|atecc_install_aes_password|AESパスワードを導入|
|2-35|atecc|atecc_generate_pubkey_from_privkey|外部秘密鍵から公開鍵を生成|
|2-36|atecc|atecc_generate_sign_with_privkey|外部秘密鍵を使用し署名|

[注1]「Persistent Latch」と呼ばれるATECC608A内のフラグ。この「Persistent Latch」を設定しないと、ATECC608AによるAES暗号化／復号化処理が実行できないようになっています。

## 仕様

#### Configバイトのカスタマイズ

ATECC608AのConfigバイト中、カスタマイズを行った箇所は下記になります。

|Slot#|SlotConfig|KeyConfig|目的|説明|
|:---:|:---:|:---:|:---|:---|
|0〜7|0x2083|0x0033|秘密鍵|カスタム秘密鍵を収容。[注1]<br>将来の仕様拡張時に利用できます。|
|8|0x0000|0x003C|データ|機密データ保管用の領域。<br>将来の仕様拡張時に利用できます。|
|9|0x2087|0x0033|秘密鍵|ECDH共通鍵生成時に利用できるカスタム秘密鍵を収容。[注1]<br>将来の仕様拡張時に利用できます。|
|10〜12|0x2083|0x0033|秘密鍵|カスタム秘密鍵を収容。[注1]<br>将来の仕様拡張時に利用できます。|
|13|0x0F8F|0x1038|AESパスワード|FIDO機能のキーハンドル／クレデンシャルIDを暗号化／復号化するために<br>必要なパスワードを収容。[注1]<br>WebAuthnユーザー登録／ログイン時に使用されます。[注2]<br>管理ツールによる秘密鍵インストール／削除の度に上書き更新されます。[注3]|
|14|0x6F83|0x0033|秘密鍵|FIDO認証器固有の秘密鍵を収容。[注1]<br>WebAuthnユーザー登録時の署名に使用されます。<br>管理ツールによる秘密鍵インストールの度に上書き更新されます。|
|15|0x0F8F|0x003C|データ|14番スロットに暗号化書込みするための一時キーを収容。[注1]<br>14番スロット更新の度に上書き更新されます。[注4]|

[注1] このスロットに書き込まれた内容は、いかなる方法によっても参照できません。<br>
[注2] WebAuthnユーザー登録時は、AESパスワードによるキーハンドル／クレデンシャルIDの暗号化が行われます。一方、WebAuthnログイン時は、AESパスワードによるキーハンドル／クレデンシャルIDの復号化が行われます。<br>
[注3] 管理ツールによる秘密鍵インストール時は、AESパスワードがすでに存在する場合は、上書き更新が行われないようになっております。一方、管理ツールによる秘密鍵削除時は、必ずAESパスワードが上書き更新されるようになっております。<br>
[注4] 14番スロットは、暗号化しないと書込みができないため、事前に暗号（一時キー）を15番スロットに書き込んでおく必要があります。また、前述「Persistent Latch」設定時は実行時に暗号が必要となるため、事前に暗号を15番スロットに書き込んでおく必要があります。14番スロット書込み、または「Persistent Latch」設定が完了すれば、15番スロットの内容は不要となります。<br>
