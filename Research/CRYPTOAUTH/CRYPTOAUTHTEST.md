# ATECC608A関数テストモジュールについて

セキュリティーIC「ATECC608A」用関数群のテストモジュールについて記載いたします。

## 概要

[ATECC608A関数群](CRYPTOAUTHFUNC.md)を単体で動作確認させるためのモジュールです。<br>
テストモジュールは「`fido_cryptoauth_test.c`」というファイルに実装されています。

下記関数`fido_cryptoauth_test_functions`の呼出で、テストが実行されます。<br>
テストモジュール内では、[ATECC608A関数群](CRYPTOAUTHFUNC.md)の関数一覧に掲載した全関数が実行されています。

```
/*
 * File:   fido_cryptoauth_test.h
 */
void fido_cryptoauth_test_functions(void);
```

使用にあたっては、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)やnRF52840 DK等に、ATECC80Aが組み込まれていることが前提となります。

## 実行方法

下記のように、MAIN SW（基板上のボタン）押下時に実行される関数に、`fido_cryptoauth_test_functions`を組み込みます。<br>
実装モジュールは`fido_command.c`になります。

```
/*
 * File:   fido_command.c
 */
 ：
// for cryptoauth function test
#include "fido_cryptoauth_test.h"
：
void fido_command_mainsw_event_handler(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_u2f_command_on_mainsw_event() == true) {
        return;
    }
    fido_ctap2_command_on_mainsw_event();

    // for CRYPTOAUTH function test <-- ここに組込みます。
    fido_cryptoauth_test_functions();
}
```

MAIN SWを押すたびに下記のテスト項目が実行されます。

- 外部秘密鍵導入テスト
- SHA-256ハッシュ生成テスト
- ECDSA署名 ＆ HMAC SHA-256ハッシュ生成テスト
- ECDH共通鍵生成テスト

ボタンを押下するごとに機能を実行するのは、一息に実行させると、nRF52840からのUARTログが途切れてしまう不具合を回避するための措置になります。

## 確認方法

ボタン押下ごとに、下記のようなUARTログが出力されますので、エラーが発生していないかどうかを確認します。

#### 外部秘密鍵導入テスト

nRF52840内のFlash ROMに導入済みの秘密鍵・証明書を使って、ATECC608Aの14番スロットに秘密鍵をインストールします。<br>
証明書に含まれる公開鍵と、14番スロットの秘密鍵から生成される公開鍵を比較し、内容が同一であれば、処理はOKです。

```
<info> app: fido_cryptoauth_init start
<info> app: fido_cryptoauth_setup_config: Config and data zone is already locked
<info> app: fido_cryptoauth_init success
<debug> app: Private key of certificate (installed into ATECC608A):
<debug> app:  CF 91 6D 82 30 03 CB 4B|..m.0..K
<debug> app:  4D C8 6A FF 05 14 49 C1|M.j...I.
<debug> app:  F4 11 FC 67 37 B7 3A 71|...g7.:q
<debug> app:  53 A3 4E 65 0D 03 95 D0|S.Ne....
<debug> app: Public key of certificate:
<debug> app:  90 99 23 34 D1 DB 65 29|..#4..e)
<debug> app:  F3 DA 89 52 42 33 FB F6|...RB3..
<debug> app:  3C A5 25 02 E3 41 91 E8|<.%..A..
<debug> app:  A5 16 6F 3F CC 32 F3 3B|..o?.2.;
<debug> app:  30 BD 98 BB 85 6F 75 85|0....ou.
<debug> app:  05 E7 81 4E 19 57 65 C2|...N.We.
<debug> app:  DF 9B BE 2B 20 7A E8 AA|...+ z..
<debug> app:  21 B2 BD FC 1B 13 77 1F|!.....w.
<debug> app: Public key from ATECC608A:
<debug> app:  90 99 23 34 D1 DB 65 29|..#4..e)
<debug> app:  F3 DA 89 52 42 33 FB F6|...RB3..
<debug> app:  3C A5 25 02 E3 41 91 E8|<.%..A..
<debug> app:  A5 16 6F 3F CC 32 F3 3B|..o?.2.;
<debug> app:  30 BD 98 BB 85 6F 75 85|0....ou.
<debug> app:  05 E7 81 4E 19 57 65 C2|...N.We.
<debug> app:  DF 9B BE 2B 20 7A E8 AA|...+ z..
<debug> app:  21 B2 BD FC 1B 13 77 1F|!.....w.
<info> app: fido_cryptoauth_release done
```

#### SHA-256ハッシュ生成テスト
32バイトのランダムベクター生成と、SHA-256ハッシュ生成のテストが行われます。<br>
SHA-256ハッシュ生成については、現行実装の「nRF Crypto」を使用した関数（`fido_crypto_generate_sha256_hash`）の実行結果と比較し、内容が同一であれば、処理はOKです。

```
<info> app: fido_cryptoauth_init start
<info> app: fido_cryptoauth_setup_config: Config and data zone is already locked
<info> app: fido_cryptoauth_init success
<debug> app: fido_cryptoauth_generate_random_vector (32 bytes):
<debug> app:  E6 3F 08 03 DD E4 E4 FB|.?......
<debug> app:  91 62 10 8C AE 23 C6 62|.b...#.b
<debug> app:  4C 5C A7 21 10 D9 E6 73|L\.!...s
<debug> app:  C7 24 4C 8C CC 6E 0F 4C|.$L..n.L
<debug> app: fido_crypto_generate_sha256_hash (32 bytes):
<debug> app:  87 EC A4 AB 4D 60 3B 99|....M`;.
<debug> app:  0A 54 DD A7 2C 3D 9E 14|.T..,=..
<debug> app:  E7 9E 19 F9 8C 80 0F BF|........
<debug> app:  33 96 5D 92 6E 28 EF B4|3.].n(..
<debug> app: fido_cryptoauth_generate_sha256_hash (32 bytes):
<debug> app:  87 EC A4 AB 4D 60 3B 99|....M`;.
<debug> app:  0A 54 DD A7 2C 3D 9E 14|.T..,=..
<debug> app:  E7 9E 19 F9 8C 80 0F BF|........
<debug> app:  33 96 5D 92 6E 28 EF B4|3.].n(..
<info> app: fido_cryptoauth_release done
```


#### ECDSA署名 ＆ HMAC SHA-256ハッシュ生成テスト
ECDSA署名生成と、HMAC-SHA-256ハッシュ生成のテストが行われます。<br>
あらかじめ0番スロットに作成した秘密鍵を使用し、ECDSA署名を生成します。<br>
HMAC-SHA-256ハッシュ生成については、現行実装の「nRF Crypto」を使用した関数（`fido_crypto_calculate_hmac_sha256`）の実行結果と比較し、内容が同一であれば、処理はOKです。

```
<info> app: fido_cryptoauth_init start
<info> app: fido_cryptoauth_setup_config: Config and data zone is already locked
<info> app: fido_cryptoauth_init success
<debug> app: fido_cryptoauth_keypair_generate success (key_id=0)
<debug> app: fido_cryptoauth_ecdsa_sign:
<debug> app:  1A 34 52 07 5B A3 78 D3|.4R.[.x.
<debug> app:  F7 41 66 68 0E 86 2E 6C|.Afh...l
<debug> app:  A5 6F E9 C3 99 58 28 A4|.o...X(.
<debug> app:  B8 32 29 8A B2 23 C2 FF|.2)..#..
<debug> app:  BE D3 79 78 48 14 1F 00|..yxH...
<debug> app:  1D 75 C6 0B ED AB BF 43|.u.....C
<debug> app:  04 14 5E 97 24 8E 5E 27|..^.$.^'
<debug> app:  BA 59 F1 49 D1 3C 16 B3|.Y.I.<..
<debug> app: fido_crypto_calculate_hmac_sha256 (32 bytes):
<debug> app:  41 6D 7D 81 7F 06 03 92|Am}.....
<debug> app:  5A 43 59 D3 0D 91 60 5F|ZCY...`_
<debug> app:  F6 CC F5 F3 04 A1 C8 5C|.......\
<debug> app:  01 82 65 B8 B6 A7 37 74|..e...7t
<debug> app: fido_cryptoauth_calculate_hmac_sha256 (32 bytes):
<debug> app:  41 6D 7D 81 7F 06 03 92|Am}.....
<debug> app:  5A 43 59 D3 0D 91 60 5F|ZCY...`_
<debug> app:  F6 CC F5 F3 04 A1 C8 5C|.......\
<debug> app:  01 82 65 B8 B6 A7 37 74|..e...7t
<info> app: fido_cryptoauth_release done
```


#### ECDH共通鍵生成テスト
ECDH共通鍵生成のテストが行われます。<br>
あらかじめ9番スロットに作成した秘密鍵を使用し、ECDH共通鍵を生成します。<br>
現行実装の「nRF Crypto」を使用した関数（`fido_crypto_sskey_generate`）の実行結果と比較し、内容が同一であれば、処理はOKです。

```
<info> app: fido_cryptoauth_init start
<info> app: fido_cryptoauth_setup_config: Config and data zone is already locked
<info> app: fido_cryptoauth_init success
<debug> app: fido_cryptoauth_keypair_generate success (key_id=9)
<debug> app: Keypair for exchanging key generate success
<debug> fido_crypto_sskey: Keypair for exchanging key is already exist
<debug> app: fido_crypto_sskey_generate:
<debug> app:  13 F5 11 DB 7C E4 FA 66|....|..f
<debug> app:  70 B8 44 E0 E1 42 80 7E|p.D..B.~
<debug> app:  6C 04 E1 96 BE E7 63 2F|l.....c/
<debug> app:  85 62 36 35 57 75 B5 E7|.b65Wu..
<debug> app: fido_cryptoauth_sskey_generate:
<debug> app:  13 F5 11 DB 7C E4 FA 66|....|..f
<debug> app:  70 B8 44 E0 E1 42 80 7E|p.D..B.~
<debug> app:  6C 04 E1 96 BE E7 63 2F|l.....c/
<debug> app:  85 62 36 35 57 75 B5 E7|.b65Wu..
<info> app: fido_cryptoauth_release done
```

## エラー発生例

#### ATECC608A未接続時

関数群内部でタイムアウトエラー（`ATCA_RX_TIMEOUT`）が発生し、下記のようなログが出力されます。

```
<info> app: fido_cryptoauth_init start
<error> fido_twi: ATECC608A hal_i2c_receive: nrf_drv_twi_rx returns 33281
<error> fido_twi: ATECC608A hal_i2c_receive: nrf_drv_twi_rx returns 33281
：
<error> fido_twi: ATECC608A hal_i2c_receive: nrf_drv_twi_rx returns 33281
<error> fido_twi: ATECC608A hal_i2c_receive: nrf_drv_twi_rx returns 33281
<error> atecc608a_i2c_hal: hal_i2c_wake failed: ATCA_RX_TIMEOUT
<error> fido_twi: ATECC608A hal_i2c_send: nrf_drv_twi_tx returns 33281
<error> app: fido_cryptoauth_init failed: atcab_init() returns 0xEB
```
