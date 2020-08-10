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

|#|関数名|説明|
|:---:|:---|:---|
|1-1|atecc_aes.c/.h|ATECC608A組込みAES関連|
|1-2|atecc_config.c/.h|各種Config関連|
|1-3|atecc_device.c/.h|デバイス制御関連|
|1-4|atecc_key.c/.h|キーペア関連|
|1-5|atecc_read.c/.h|各種データ照会関連|
|1-6|atecc_util.c/.h|各種ユーティリティー関連|
|1-7|atecc_write.c/.h|各種データ更新関連|

#### 関数一覧

移植作業中に追加／削除される可能性があります。

|#|関数名|説明|
|:---:|:---|:---|
|2-1|atecc_init|デバイス初期化|
|2-2|atecc_release|デバイス解放|
|2-3|atecc_is_locked|デバイスロック状態取得|
|2-4|atecc_lock_config_zone|Config領域ロック|
|2-5|atecc_lock_data_zone|データ領域ロック|
|2-6|atecc_write_config_zone|Config領域更新|
|2-7|atecc_write_zone|データ領域更新|
|2-8|atecc_read_config_zone|Config領域照会|
|2-9|atecc_read_zone|データ領域照会|
|2-10|atecc_read_serial_number|シリアル番号照会|
|2-11|atecc_random|ランダム値生成|
|2-12|atecc_nonce_load|NONCE値照会|
|2-13|atecc_priv_write|秘密鍵更新|
|2-14|atecc_genkey|キーペア生成|
|2-15|atecc_get_pubkey|公開鍵照会|
|2-16|atecc_aes_cbc_init|AES初期化|
|2-17|atecc_aes_cbc_encrypt_block|AES暗号化|
|2-18|atecc_aes_cbc_decrypt_block|AES復号化|

## 仕様

（後報）
