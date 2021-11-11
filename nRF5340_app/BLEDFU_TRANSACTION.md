# BLE DFU機能のトランザクション

最終更新日：2021/11/11

## 概要

nRF Connect SDK（Zephyrプラットフォーム）で提供しているBLE経由のファームウェア更新機能（[BLE DFU機能](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/dfu.html#mcuboot)）について、処理中に発生するトランザクションの情報を掲載しています。

#### 実行したツール
[BLE DFU機能](../nRF5340_app/BLEDFU_FUNC_OBJC.md)を実装した[macOS版管理ツール](../MaintenanceTool/macOSApp)を使用しています。

#### 更新したアプリケーション
[Zephyrアプリケーション](../nRF5340_app/secure_device_app)のバージョン文字列を`0.4.0`-->`0.4.1`に更新してDFUを実行しています。

## データについて

トランザクションデータは、ヘッダーと本体で構成されます。

#### ヘッダー
８バイト固定になります。<br>
以下の表に、本ドキュメントで登場する内容を掲載しています。

|Offset|Keyword|Type|内容|
|:--:|:-|:-|:-|
|0|`operation`|`uint8_t`|`0`:`READ(request)`、`1`:`READ(response)`、<br>`2`:`WRITE(request)`、`3`:`WRITE(response)`|
|1|`flags`|`uint8_t`|`0`固定|
|2|`length`|`uint16_t`|本体のバイト数|
|4|`group`|`uint16_t`|`0`:`OS`、`1`:`IMAGE`|
|6|`sequence`|`uint8_t`|`0`固定|
|7|`command id`|`uint8_t`|`group=0`の場合、`5`:`RESET`<br>`group=1`の場合、`0`:`STATE`、`1`:`UPLOAD`|

#### 本体
可変長のデータで、CBOR形式となっています。

例として、以下は`SMP response`のデータサンプルになります。

```
Transmit SMP response (142 bytes)
01 00 00 86 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 94 03 62
7d 89 6c 26 33 a3 0c 54 ba 31 85 11 87 94 03 0a
54 81 75 3c 5c 66 f3 2c 4b fd f8 85 b0 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff ff
6b 73 70 6c 69 74 53 74 61 74 75 73 00 ff
```

下記は`SMP response`本体のCBORデコードイメージです。

```
bf
    66 696d61676573
    9f
        bf
            64 736c6f74 00
            67 76657273696f6e 65 302e302e30
            64 68617368 5820 9403627d896c2633a30c54ba3185118794030a5481753c5c66f32c4bfdf885b0
            68 626f6f7461626c65 f5
            67 70656e64696e67 f4
            69 636f6e6669726d6564 f5
            66 616374697665 f5
            69 7065726d616e656e74 f4
        ff
    ff
    6b 73706c6974537461747573 00
ff
```

上記のテキストデコードイメージは下記のようになります。<br>
（`hash`は、Base64エンコードしたものです。実際は32バイトのバイナリーデータです）

```
{
    "images":[
        {
            "slot":0,
            "version":"0.0.0",
            "hash":"lANifYlsJjOjDFS6MYURh5QDClSBdTxcZvMsS/34hbA=",
            "bootable":true,
            "pending":false,
            "confirmed":true,
            "active":true,
            "permanent":false
        }
    ],
    "splitStatus":0
}
```


## トランザクションのログ

以下のトランザクションで出力されるログについて掲載します。

- ファームウェア更新イメージの転送
  - スロット照会
  - 転送実行

- ファームウェア更新イメージの反映
  - 反映要求
  - リセット要求

### スロット照会

nRF5340のFlash ROM領域（スロット）の情報を照会します。

#### SMPリクエスト（管理ツール-->認証器、以下同）

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`0`:`READ(request)`|`0`|`2`|`1`:`IMAGE`|`0`|`0`:`STATE`|

本体はブランクです。<br>
（ブランクであっても、開始、終了文字である`bf ff`が出力されるので、本体は２バイトになります）

#### SMPレスポンス（認証器-->管理ツール、以下同）

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`1`:`READ(response)`|`0`|本体のバイト数|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### データサンプル

```
Transmit SMP request (10 bytes)
00 00 00 02 00 01 00 00 bf ff
Incoming SMP response (142 bytes)
01 00 00 86 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 2f 21 8a
7d 24 4c 69 2c a5 96 c4 5d 7d 10 d2 ad 8f 97 62
86 17 a0 3d 21 22 9d fa 25 73 b6 47 cc 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff ff
6b 73 70 6c 69 74 53 74 61 74 75 73 00 ff
```

#### 参考：テキストデコードイメージ

【レスポンス】<br>
ファームウェア更新イメージが書き込まれる前ですので、スロット#1の情報はありません。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0",
            "hash":2f218a7d244c692ca596c45d7d10d2ad8f97628617a03d21229dfa2573b647cc,
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        }
    ],
    "splitStatus":0
}
```

### 転送実行

ファームウェア更新イメージを、認証器に分割転送します。

#### SMPリクエスト

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`2`:`WRITE(request)`|`0`|本体のバイト数[注1]|`1`:`IMAGE`|`0`|`1`:`UPLOAD`|

#### SMPレスポンス

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`3`:`WRITE(response)`|`0`|本体のバイト数|`1`:`IMAGE`|`0`|`1`:`UPLOAD`|

#### データサンプル

先頭パケット（分割送信の１回目）のサンプルです。

```
Transmit SMP request (248 bytes)
02 00 00 f0 00 01 00 01 bf 63 6c 65 6e 1a 00 03
ad 6c 63 73 68 61 43 25 19 c6 63 6f 66 66 00 64
64 61 74 61 58 d1 3d b8 f3 96 00 00 00 00 00 02
00 00 1c aa 03 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff
Incoming SMP response (20 bytes)
03 00 00 0c 00 01 00 01 bf 62 72 63 00 63 6f 66
66 18 d0 ff
```

#### 参考：テキストデコードイメージ

【リクエスト】<br>
転送データ長は`d1`（209バイト）、総データ長は`03ad6c`（241,004バイト）です。<br>
SHA値は、ファームウェア更新イメージファイルから算出したハッシュ値の先頭３バイトのみ抽出しているようです。[注2]

```
{   
    "data":3db8f39600000000000200001caa0300...ffffffffffffffffffffffffffffffff,
    "len":03ad6c,
    "sha":2519c6,
    "off":0
}
```

【レスポンス】<br>
リターンコード、オフセット[注3]が戻るようです。

```
{
    "rc":0,"off":d0
}
```

[注1] 本体のバイト数上限は`f0`（240バイト）となっています。<br>
[注2] 調査に使用したファームウェア更新イメージファイル`app_update.PCA10095.0.4.1.bin`から、コマンド`shasum -a 256`により計算されたSHA-256ハッシュ値は`2519c66e55efd27bd416379fb3d3e0ec8dfd11ca65f95fe788dc4ddb6aac60f9`でした。<br>
[注3] 次回転送時に要求する転送データの先頭インデックス（通常、書き込んだ通算バイト数の値）になります。概ね８の倍数になる事が多いようです。

### 反映要求

認証器のFlash ROM領域（スロット）に書き込まれているファームウェア更新イメージを、リセット後に反映させるよう、Zephyrアプリケーションに指示します。

#### SMPリクエスト

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`2`:`WRITE(request)`|`0`|`50`|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### SMPレスポンス

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`3`:`WRITE(response)`|`0`|本体のバイト数|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### データサンプル

```
Transmit SMP request (58 bytes)
02 00 00 32 00 01 00 00 bf 67 63 6f 6e 66 69 72
6d f5 64 68 61 73 68 58 20 ad 5b 7e cc 96 c9 15
90 91 ea 47 98 b7 64 a4 8c ec a6 c8 fb 11 b1 d9
e4 5d 3e c1 20 77 1d a3 b8 ff
Incoming SMP response (249 bytes)
03 00 00 f4 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 2f 21 8a
7d 24 4c 69 2c a5 96 c4 5d 7d 10 d2 ad 8f 97 62
86 17 a0 3d 21 22 9d fa 25 73 b6 47 cc 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff bf
64 73 6c 6f 74 01 67 76 65 72 73 69 6f 6e 65 30
2e 30 2e 30 64 68 61 73 68 58 20 ad 5b 7e cc 96
c9 15 90 91 ea 47 98 b7 64 a4 8c ec a6 c8 fb 11
b1 d9 e4 5d 3e c1 20 77 1d a3 b8 68 62 6f 6f 74
61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f5 69 63
6f 6e 66 69 72 6d 65 64 f4 66 61 63 74 69 76 65
f4 69 70 65 72 6d 61 6e 65 6e 74 f5 ff ff 6b 73
70 6c 69 74 53 74 61 74 75
2021-11-05 15:17:09.048 [debug] Incoming SMP response (3 bytes)
73 00 ff
```

#### 参考：テキストデコードイメージ

【リクエスト】

```
{
    "confirm":true,
    "hash":ad5b7ecc96c9159091ea4798b764a48ceca6c8fb11b1d9e45d3ec120771da3b8
}
```

【レスポンス】<br>
スロット#1のイメージは、`permanent=true`となっています。<br>
この場合は、次回以降のパワーオン／リセット時、スロット#1のイメージ（更新後のファームウェアイメージ）がブートされます。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0",
            "hash":2f218a7d244c692ca596c45d7d10d2ad8f97628617a03d21229dfa2573b647cc,
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0",
            "hash":ad5b7ecc96c9159091ea4798b764a48ceca6c8fb11b1d9e45d3ec120771da3b8,
            "bootable":true,"pending":true,"confirmed":false,"active":false,"permanent":true
        }
    ],
    "splitStatus":0
}
```

### リセット要求

nRF5340にリセット実行（アプリケーションの再起動）を要求します。

#### SMPリクエスト

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`2`:`WRITE(request)`|`0`|`2`|`0`:`OS`|`0`|`5`:`RESET`|

#### SMPレスポンス

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`3`:`WRITE(response)`|`0`|`2`|`0`:`OS`|`0`|`5`:`RESET`|


#### データサンプル

```
Transmit SMP request (10 bytes)
02 00 00 02 00 00 00 05 bf ff
Incoming SMP response (10 bytes)
03 00 00 02 00 00 00 05 bf ff
```

## （ご参考）CBOR様式の差異について

BLE DFU機能では、前述の通りCBORというデータ形式を使用しています。<br>
下記に詳しい仕様記述があります。

- <b>RFC 8949 - Concise Binary Object Representation (CBOR)</b><br>
 [https://www.rfc-editor.org/rfc/rfc8949.html](https://www.rfc-editor.org/rfc/rfc8949.html)

CBOR自体は、FIDO機能でも使用されていますが、配列データ[注1]のデータエンコード／デコード形式に差異があります。

#### FIDO機能のCBORエンコード

`definite-length encoding`という形式を使用しています。

配列に含まれる要素数が決まっている（固定数の）場合に使用します。<br>
下記は`[1, [2, 3], [4, 5]]`という配列をエンコードしたものです。

```
83        -- Array of length 3
   01     -- 1
   82     -- Array of length 2
      02  -- 2
      03  -- 3
   82     -- Array of length 2
      04  -- 4
      05  -- 5
```

#### BLE DFU機能のCBORエンコード

`indefinite-length encoding`という形式を使用しています。

配列に含まれる要素数が決まっていない（可変数の）場合に使用します。<br>
下記は`{"Fun":true, "Amt":-2}`という連想配列をエンコードしたものです。

```
BF           -- Start indefinite-length map
   63        -- First key, UTF-8 string length 3
      46756e --   "Fun"
   F5        -- First value, true
   63        -- Second key, UTF-8 string length 3
      416d74 --   "Amt"
   21        -- Second value, -2
FF           -- "break"
```

[注1] 配列データには、Map（Key／Valueを持つ連想配列）、Array（単純配列）などがあります。
