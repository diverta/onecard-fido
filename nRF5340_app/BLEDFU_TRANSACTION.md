# BLE DFU機能のトランザクション調査

最終更新日：2021/10/12

## 概要

nRF Connectで提供しているBLE経由のファームウェア更新機能（[BLE DFU機能](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/dfu.html#mcuboot)）について、処理中に発生するトランザクションの調査・解析を行います。

#### 実行したツール
ログ採取が可能なツールである、Nordic社提供の「[nRF Connectアプリ](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Connect-for-mobile)」を使用しています。

#### 更新したアプリケーション
[nRF5340アプリケーション](../nRF5340_app/secure_device_app)のバージョン文字列を`0.4.0`-->`0.4.1`に更新してDFUを実行しています。

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
  - スロット照会（転送前）
  - 転送実行
  - 反映一時停止要求
  - リセット要求


- ファームウェア更新イメージの反映
  - スロット照会（反映前）
  - 反映要求

### スロット照会（転送前）

nRF5340のFlash ROM領域（スロット）の情報を照会します。

#### SMPリクエスト（Android-->nRF5340、以下同）

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`0`:`READ(request)`|`0`|`2`|`1`:`IMAGE`|`0`|`0`:`STATE`|

本体はブランクです。<br>
（ブランクであっても、開始、終了文字である`bf ff`が出力されるので、本体は２バイトになります）

#### SMPレスポンス（nRF5340-->Android、以下同）

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`1`:`READ(response)`|`0`|本体のバイト数|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### データサンプル

```
Incoming SMP request (10 bytes)
00 00 00 02 00 01 00 00 bf ff
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

#### 参考：テキストデコードイメージ

【レスポンス】<br>
ファームウェア更新イメージが書き込まれる前ですので、スロット#1の情報はありません。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"lANifYlsJjOjDFS6MYURh5QDClSBdTxcZvMsS/34hbA=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        }
    ],
    "splitStatus":0
}
```

### 転送実行

ファームウェア更新イメージを、nRF5340に分割転送します。

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
Incoming SMP request (248 bytes)
02 00 00 f0 00 01 00 01 bf 64 64 61 74 61 58 d1
3d b8 f3 96 00 00 00 00 00 02 00 00 c4 a9 03 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff 63 6c 65 6e 1a 00 03 ad 14 63 73 68 61 43 88
fe 49 63 6f 66 66 00 ff
Transmit SMP response (20 bytes)
03 00 00 0c 00 01 00 01 bf 62 72 63 00 63 6f 66
66 18 d0 ff
```

#### 参考：テキストデコードイメージ

【リクエスト】<br>
転送データ長は`d1`（209バイト）、総データ長は`03ad14`（240,916バイト）です。<br>
SHA値は、ファームウェア更新イメージファイルから算出したハッシュ値の先頭３バイトのみ抽出しているようです。[注2]

```
{   
    "data":3db8f3960000000000020000c4a90300...ffffffffffffffffffffffffffffffff,
    "len":03ad14,
    "sha":88fe49,
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
[注2] 調査に使用したファームウェア更新イメージファイル`app_update.PCA10095.0.4.1.bin`から計算されたSHA-256ハッシュ値は`88fe494d209728471713d6e98ba4ef7a55fd02d40a83349207851232f89e2bd4`でした。<br>
[注3] 次回転送時に要求する転送データの先頭インデックス（通常、書き込んだ通算バイト数の値）になります。概ね８の倍数になる事が多いようです。

### 反映一時停止要求

nRF5340のFlash ROM領域（スロット）に書き込まれているファームウェア更新イメージを、一時的に反映しないようにします。

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
Incoming SMP request (58 bytes)
02 00 00 32 00 01 00 00 bf 67 63 6f 6e 66 69 72
6d f4 64 68 61 73 68 58 20 82 c3 bf 4f ae 87 5b
c1 84 44 c7 88 05 92 d4 2a a1 69 9f 96 f3 3e 24
23 2c c4 a6 e5 be ba 38 52 ff
Transmit SMP response (249 bytes)
03 00 00 f4 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 94 03 62
7d 89 6c 26 33 a3 0c 54 ba 31 85 11 87 94 03 0a
54 81 75 3c 5c 66 f3 2c 4b fd f8 85 b0 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff bf
64 73 6c 6f 74 01 67 76 65 72 73 69 6f 6e 65 30
2e 30 2e 30 64 68 61 73 68 58 20 82 c3 bf 4f ae
87 5b c1 84 44 c7 88 05 92 d4 2a a1 69 9f 96 f3
3e 24 23 2c c4 a6 e5 be ba 38 52 68 62 6f 6f 74
61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f5 69 63
6f 6e 66 69 72 6d 65 64 f4 66 61 63 74 69 76 65
f4 69 70 65 72 6d 61 6e 65 6e 74 f4 ff ff 6b 73
70 6c 69 74 53 74 61 74 75
Transmit SMP response (3 bytes)
73 00 ff
```

#### 参考：テキストデコードイメージ

【リクエスト】

```
{   
    "confirm":false,
    "hash":"gsO/T66HW8GERMeIBZLUKqFpn5bzPiQjLMSm5b66OFI="
}
```

【レスポンス】<br>
ファームウェア更新イメージが書き込まれたスロット#1は、`active=false, confirmed=false`の状態です。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"lANifYlsJjOjDFS6MYURh5QDClSBdTxcZvMsS/34hbA=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"gsO/T66HW8GERMeIBZLUKqFpn5bzPiQjLMSm5b66OFI=",
            "bootable":true,"pending":true,"confirmed":false,"active":false,"permanent":false
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

### スロット照会（反映前）

nRF5340のFlash ROM領域（スロット）の情報を照会します。

#### SMPリクエスト

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`0`:`READ(request)`|`0`|`2`|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### SMPレスポンス

|`operation`|`flags`|`length`|`group`|`sequence`|`command id`|
|:-:|:-:|:-:|:-:|:-:|:-:|
|`1`:`READ(response)`|`0`|本体のバイト数|`1`:`IMAGE`|`0`|`0`:`STATE`|

#### データサンプル

```
Incoming SMP request (10 bytes)
00 00 00 02 00 01 00 00 bf ff
Transmit SMP response (249 bytes)
01 00 00 f4 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 82 c3 bf
4f ae 87 5b c1 84 44 c7 88 05 92 d4 2a a1 69 9f
96 f3 3e 24 23 2c c4 a6 e5 be ba 38 52 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f4 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff bf
64 73 6c 6f 74 01 67 76 65 72 73 69 6f 6e 65 30
2e 30 2e 30 64 68 61 73 68 58 20 94 03 62 7d 89
6c 26 33 a3 0c 54 ba 31 85 11 87 94 03 0a 54 81
75 3c 5c 66 f3 2c 4b fd f8 85 b0 68 62 6f 6f 74
61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4 69 63
6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69 76 65
f4 69 70 65 72 6d 61 6e 65 6e 74 f4 ff ff 6b 73
70 6c 69 74 53 74 61 74 75
Transmit SMP response (3 bytes)
73 00 ff
```

#### 参考：テキストデコードイメージ

【レスポンス】<br>
前述「リセット要求」後のリセットにより、スロット#0、#1の内容が入れ替わっている事がわかります。

ファームウェア更新イメージが書き込まれたスロット#0は、`active=true, confirmed=false`の状態です。<br>
この状態ですと、次回パワーオン／リセット時は、更新前のファームウェアイメージがブートされ、結果として、更新が取り消されることとなります。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0",
            "hash":"gsO/T66HW8GERMeIBZLUKqFpn5bzPiQjLMSm5b66OFI=",
            "bootable":true,"pending":false,"confirmed":false,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0",
            "hash":"lANifYlsJjOjDFS6MYURh5QDClSBdTxcZvMsS/34hbA=",
            "bootable":true,"pending":false,"confirmed":true,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
```

### 反映要求

nRF5340のFlash ROM領域（スロット）に書き込まれているファームウェア更新イメージを反映します。

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
Incoming SMP request (58 bytes)
02 00 00 32 00 01 00 00 bf 67 63 6f 6e 66 69 72
6d f5 64 68 61 73 68 58 20 82 c3 bf 4f ae 87 5b
c1 84 44 c7 88 05 92 d4 2a a1 69 9f 96 f3 3e 24
23 2c c4 a6 e5 be ba 38 52 ff
Transmit SMP response (249 bytes)
03 00 00 f4 00 01 00 00 bf 66 69 6d 61 67 65 73
9f bf 64 73 6c 6f 74 00 67 76 65 72 73 69 6f 6e
65 30 2e 30 2e 30 64 68 61 73 68 58 20 82 c3 bf
4f ae 87 5b c1 84 44 c7 88 05 92 d4 2a a1 69 9f
96 f3 3e 24 23 2c c4 a6 e5 be ba 38 52 68 62 6f
6f 74 61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4
69 63 6f 6e 66 69 72 6d 65 64 f5 66 61 63 74 69
76 65 f5 69 70 65 72 6d 61 6e 65 6e 74 f4 ff bf
64 73 6c 6f 74 01 67 76 65 72 73 69 6f 6e 65 30
2e 30 2e 30 64 68 61 73 68 58 20 94 03 62 7d 89
6c 26 33 a3 0c 54 ba 31 85 11 87 94 03 0a 54 81
75 3c 5c 66 f3 2c 4b fd f8 85 b0 68 62 6f 6f 74
61 62 6c 65 f5 67 70 65 6e 64 69 6e 67 f4 69 63
6f 6e 66 69 72 6d 65 64 f4 66 61 63 74 69 76 65
f4 69 70 65 72 6d 61 6e 65 6e 74 f4 ff ff 6b 73
70 6c 69 74 53 74 61 74 75
Transmit SMP response (3 bytes)
73 00 ff
```

#### 参考：テキストデコードイメージ

【リクエスト】

```
{
    "confirm":true,
    "hash":"gsO/T66HW8GERMeIBZLUKqFpn5bzPiQjLMSm5b66OFI="
}
```

【レスポンス】<br>
スロット#0が、`active=true, confirmed=true`の状態に遷移しています。<br>
この場合は、次回以降のパワーオン／リセット時も、更新後のファームウェアイメージがブートされます。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"gsO/T66HW8GERMeIBZLUKqFpn5bzPiQjLMSm5b66OFI=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"lANifYlsJjOjDFS6MYURh5QDClSBdTxcZvMsS/34hbA=",
            "bootable":true,"pending":false,"confirmed":false,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
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
