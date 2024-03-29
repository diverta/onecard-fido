# 詳細仕様

最終更新日：2022/11/23

## 概要
OATHに関する仕様の詳細について記載しています。

## コマンド仕様

以下のコマンドについてのリクエスト／レスポンスに関する仕様を記載しています。
- 認証器にクレデンシャルを個別登録
- 認証器からクレデンシャルを個別削除
- アカウント名一覧抽出

### 認証器にクレデンシャルを個別登録

<b>リクエストヘッダー</b>

|#|INS|P1|P2|名称|説明|
|:---:|:---:|:---:|:---:|:---|:---|
|1|`0x01`|`0x00`|`0x00`|`OATH_INS_PUT`|認証器にクレデンシャルを個別登録|

<b>リクエストデータ</b><br>
TLV形式で構成されています。

|#|Offset|桁数|名称または値|説明|
|:---:|:---:|:---:|:---|:---|
|1|0|1|`OATH_TAG_NAME`（`0x71`）|項目＝アカウント|
|2|1|1|データサイズ|アカウント名の長さ|
|3|2|64|アカウント名|アカウント名|
|4|66|1|`OATH_TAG_KEY`（`0x73`）|項目＝Secret|
|5|67|1|データサイズ|Secretの長さ|
|6|68|66|secret|ワンタイムパスワードを計算するための<br>HMAC暗号を保持[注1]|
|7|134|1|`OATH_TAG_PROPERTY`（`0x78`）|項目＝オプション属性|
|8|135|1|データサイズ|オプション属性の長さ|
|9|136|1|property|オプション属性|
|10|137|1|`OATH_TAG_COUNTER`（`0x7A`）|項目＝カウンター|
|11|138|1|データサイズ|カウンターの長さ|
|12|139|4|カウンター|ワンタイムパスワード計算時に使用する値|

[注1]バイトエンコードされたデータを保持。HMAC-SHAハッシュ計算時に使用します。

<b>レスポンスステータスワード</b>

|#|値|名称|説明|
|:---:|:---:|:---|:---|
|1|`0x9000`|`SW_NO_ERROR`|正常終了|
|2|上記以外|-|処理失敗|

### 認証器からクレデンシャルを個別削除

<b>リクエストヘッダー</b>

|#|INS|P1|P2|名称|説明|
|:---:|:---:|:---:|:---:|:---|:---|
|1|`0x02`|`0x00`|`0x00`|`OATH_INS_DELETE`|認証器に登録されているクレデンシャルを個別削除|

<b>リクエストデータ</b><br>
TLV形式で構成されています。

|#|Offset|桁数|名称または値|説明|
|:---:|:---:|:---:|:---|:---|
|1|0|1|`OATH_TAG_NAME`（`0x71`）|項目＝アカウント|
|2|1|1|データサイズ|アカウント名の長さ|
|3|2|64|アカウント名|削除対象アカウント名|

<b>レスポンスステータスワード</b>

|#|値|名称|説明|
|:---:|:---:|:---|:---|
|1|`0x9000`|`SW_NO_ERROR`|正常終了|
|2|上記以外|-|処理失敗|

### アカウント名一覧抽出

<b>リクエストヘッダー</b>

|#|INS|P1|P2|名称|説明|
|:---:|:---:|:---:|:---:|:---|:---|
|1|`0x03`|`0x00`|`0x00`|`OATH_INS_LIST`|認証器に登録されているアカウント名の一覧を抽出|

<b>レスポンスステータスワード</b>

|#|値|名称|説明|
|:---:|:---:|:---|:---|
|1|`0x9000`|`SW_NO_ERROR`|正常終了|
|2|上記以外|-|処理失敗|

<b>レスポンスデータ</b><br>

TLV形式で構成される以下のレイアウトのアカウントデータが、単純にアカウント数分連結されます。

|#|Offset|桁数|名称または値|説明|
|:---:|:---:|:---:|:---|:---|
|1|0|1|`OATH_TAG_NAME`（`0x71`）|項目＝アカウント|
|2|1|1|データサイズ|アカウント名の長さ|
|3|2|64|アカウント名|OATHユーザーを識別する名称|
|4|66|1|`OATH_TAG_META`（`0x75`）|項目＝メタデータ|
|5|67|1|データサイズ|メタデータの長さ（`2`）|
|6|68|1|アルゴリズム|OATH機能のアルゴリズムを`uint8_t`形式で保持[注1]|
|7|69|1|OTP桁数|ワンタイムパスワードの桁数|

[注1]上４ビット＝機能種別（`1`:HOTP、`2`:TOTP）、下４ビット＝ハッシュアルゴリズム（`1`:SHA-1、`2`:SHA-256）

### ワンタイムパスワードを計算

<b>リクエストヘッダー</b>

|#|INS|P1|P2|名称|説明|
|:---:|:---:|:---:|:---:|:---|:---|
|1|`0x04`|`0x00`|`0x00`|`OATH_INS_CALCULATE`|ワンタイムパスワードを計算|

<b>リクエストデータ</b><br>
TLV形式で構成されています。

|#|Offset|桁数|名称または値|説明|
|:---:|:---:|:---:|:---|:---|
|1|0|1|`OATH_TAG_NAME`（`0x71`）|項目＝アカウント|
|2|1|1|データサイズ|アカウント名の長さ|
|3|2|64|アカウント名|計算対象アカウント名|
|4|66|1|`OATH_TAG_CHALLENGE`（`0x71`）|項目＝チャレンジ|
|5|67|1|データサイズ|チャレンジの長さ|
|6|68|8|チャレンジ|ワンタイムパスワードを計算するための任意値|

<b>レスポンスステータスワード</b>

|#|値|名称|説明|
|:---:|:---:|:---|:---|
|1|`0x9000`|`SW_NO_ERROR`|正常終了|
|2|`0x6984`|`SW_DATA_INVALID`|アカウントが認証器に登録されていない|
|3|上記以外|-|処理失敗|

<b>レスポンスデータ</b><br>
TLV形式で構成されています。

|#|Offset|桁数|名称または値|説明|
|:---:|:---:|:---:|:---|:---|
|1|0|1|`OATH_TAG_RESPONSE`（`0x76`）|項目＝レスポンス|
|2|1|1|データサイズ|レスポンスデータの長さ（`5`）|
|3|2|1|OTP桁数|ワンタイムパスワードの桁数|
|4|3|4|ワンタイムパスワード値|計算結果をバイト配列形式で格納<br>（バイトオーダー＝ビッグエンディアン）[注1]|

[注1]このレスポンスを受け取ったOATH-TOTPクライアントは、レスポンスのビッグエンディアン形式バイト配列を数値変換（`totp_src_int`）したのち、下６桁の数値を抽出（`totp_src_int % 1000000`）し、６桁の「ワンタイムパスワード」として文字列表示することになります。
