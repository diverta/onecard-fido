# FIDO機能　ステータスワード一覧

## 概要
FIDO機能で設定しているステータスワードの一覧を掲載いたします。<br>

（2018/04/10改訂）

## 一覧（アライアンス制定要件）

|処理 |エラーコード  |内容  |
|:--|:-:|---|
|ble_u2f_command |0xBF01  |リクエストヘッダのチェックがNG (CMDが不正) |
|ble_u2f_command |0xBF03  |リクエストヘッダのチェックがNG (LENが不正) |
|ble_u2f_command |0xBF04  |リクエストヘッダのチェックがNG (SEQが不正) |
|ble_u2f_command |0x6D00  |リクエストデータ解析時、リクエストのINSチェックがNG |
|ble_u2f_command |0x6E00  |リクエストデータ解析時、リクエストのCLAチェックがNG |
|ble_u2f_command |0x6700  |リクエストデータ解析時、APDUデータ／データ長の関連チェックがNG |
|ble_u2f_version |0x6700  |U2F Versionレスポンスが6バイトでない |
|ble_u2f_register |0x6700  |U2F Registerでのレスポンスメッセージ生成時に、生成されたメッセージ長のチェックがNG |
|ble_u2f_authenticate |0x6700  |U2F Authenticateでのレスポンスメッセージ生成時に、生成されたメッセージ長のチェックがNG |
|ble_u2f_authenticate |0x6A80  |U2F Authenticate時に、キーハンドルからのappIDHash取得に失敗 |
|ble_u2f_authenticate |0x6A80  |U2F Authenticate時に、appIdHashに対応するトークンカウンターが存在しない |
|ble_u2f_authenticate |0x6985  |U2F Authenticate時に、ユーザー所在確認不要と指定された |

ステータスワードが重複しているものがありますが、これはFIDOアライアンス制定要件に準拠させるため、ユニーク化しておりません。<br>
例えば、メッセージ長がアライアンス制定要件に反している場合は、一律で `0x6700` というステータスワードを戻す必要があります。

#### ご参考
- [BLEプロトコルエラーコードについてのFIDOアライアンス制定要件](https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-bt-protocol-v1.2-ps-20170411.html#command-status-and-error-constants) <br>
`0xbfXX` 形式のものが、BLEプロトコルエラーコードになります。

- [U2FエラーステータスワードについてのFIDOアライアンス制定要件](https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-raw-message-formats-v1.2-ps-20170411.html#status-codes) <br>
`0x6XXX` 形式のものが、U2Fエラーステータスワードになります。

## 一覧（アライアンス制定要件外の個別エラー）

ステータスワードは、障害原因の切り分けを容易にするため、処理／エラーケースごとにユニーク化しています。

|処理 |エラーコード  |内容  |
|:--|:-:|---|
|ble_u2f_pairing |0x9101  |ペアリング情報を削除時、Peer Managerで内部エラーが発生 |
|ble_u2f_pairing |0x9102  |ペアリング情報を削除時、Peer Managerで削除処理が失敗 |
|ble_u2f_securekey |0x9201  |秘密鍵／証明書をFlash ROM領域から削除時に、FDSの呼出が失敗 |
|ble_u2f_securekey |0x9202  |秘密鍵／証明書をFlash ROM領域から削除時に、FDSでエラー発生 |
|ble_u2f_securekey |0x9203  |AES秘密鍵生成処理が失敗 |
|ble_u2f_securekey |0x9204  |FDS GC実行後に行われたAES秘密鍵生成処理が失敗 |
|ble_u2f_securekey |0x9311  |秘密鍵インストール時に、リクエストデータチェックがNG |
|ble_u2f_securekey |0x9312  |秘密鍵インストール時に、Flash ROMからの登録済みデータ読込失敗 |
|ble_u2f_securekey |0x9313  |秘密鍵インストール時に、リクエストデータのバイト変換失敗 |
|ble_u2f_securekey |0x9314  |秘密鍵インストール時に、FDSの呼出が失敗 |
|ble_u2f_securekey |0x9315  |秘密鍵インストール時に、FDSでエラー発生 |
|ble_u2f_securekey |0x9321  |証明書インストール時に、リクエストデータチェックがNG |
|ble_u2f_securekey |0x9322  |証明書インストール時に、Flash ROMからの登録済みデータ読込失敗 |
|ble_u2f_securekey |0x9323  |証明書インストール時に、証明書データのワード数チェックがNG |
|ble_u2f_securekey |0x9324  |証明書インストール時に、FDSの呼出が失敗 |
|ble_u2f_securekey |0x9325  |証明書インストール時に、FDSでエラー発生 |
|ble_u2f_register |0x9401  |U2F Register時に、Flash ROMからの登録済みデータ読込失敗 |
|ble_u2f_register |0x9402  |U2F Register時に、秘密鍵／証明書のFlash ROM登録チェックが失敗|
|ble_u2f_register |0x9403  |U2F Registerでのトークンカウンター追加時に、FDSの呼出が失敗 |
|ble_u2f_register |0x9404  |U2F Registerでのトークンカウンター追加時に、FDSでエラー発生 |
|ble_u2f_register |0x9405  |U2F Registerでのレスポンスメッセージ生成時に、予期しないエラーが発生 |
|ble_u2f_authenticate |0x9501  |U2F Authenticate時に、Flash ROMからの登録済みデータ読込失敗  |
|ble_u2f_authenticate |0x9502  |U2F Authenticateでのトークンカウンター更新時に、FDSの呼出が失敗 |
|ble_u2f_authenticate |0x9503  |U2F Authenticateでのトークンカウンター更新時に、FDSでエラー発生 |
|ble_u2f_authenticate |0x9504  |U2F Authenticateでのレスポンスメッセージ生成時に、予期しないエラーが発生 |

#### エラーコードの命名規約

エラーコードの先頭バイトは、下表のようのU2F管理ツールにおける処理（＝ボタン／メニューアイテム）単位といたします。

|U2F管理ツール上の処理名 |エラーコード先頭バイト  |備考  |
|:--|:-:|---|
|ペアリング情報消去 |0x91  | |
|鍵・証明書・キーハンドル消去 |0x92  | |
|鍵・証明書ファイルのインストール |0x93  | |
|U2F Register |0x94  | Chromeエクステンション、ヘルスチェック処理から呼び出されます |
|U2F Authenticate |0x95  | Chromeエクステンション、ヘルスチェック処理から呼び出されます |

また、エラーコードの末尾バイトは、処理単位ごとの連番とします。
