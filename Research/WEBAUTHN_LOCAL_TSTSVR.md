# WebAuthnローカルテストサーバー構築手順

GitHubで公開されている、WebAuthnのローカルテストサーバーをPC上に構築する手順を掲載しています。

手順は以下のページを参考にしております。<br>
https://github.com/duo-labs/webauthn#webauthn-demo

## GO環境の導入

[こちらのダウンロードページ](https://golang.org/dl/)から、インストーラーをダウンロードします。<br>
（今回の検証では、macOS版を使用しております）

<img src="assets01/0010.png" width="700">

ダウンロードしたインストーラーを実行し、GO環境を導入します。

<img src="assets01/0011.png" width="500">

インストールが完了したら、任意のフォルダーに下記のようなコードを記述したファイルを配置します。

```
package main

import "fmt"

func main() {
    fmt.Printf("hello, world\n")
}
```

ソースコードが配置されているディレクトリーに移動し、`go build`を実行してビルドします。<br>
`test`という実行可能ファイルができます。

```
MacBookPro-makmorit-jp:test makmorit$ ls -al
total 16
drwxr-xr-x   3 makmorit  staff  102  5 14 12:40 .
drwx------+ 11 makmorit  staff  374  5 14 12:39 ..
-rw-r--r--@  1 makmorit  staff   77  5 14 12:39 hello.go
MacBookPro-makmorit-jp:test makmorit$ go build
MacBookPro-makmorit-jp:test makmorit$ ls -al
total 4120
drwxr-xr-x   4 makmorit  staff      136  5 14 12:40 .
drwx------+ 11 makmorit  staff      374  5 14 12:39 ..
-rw-r--r--@  1 makmorit  staff       77  5 14 12:39 hello.go
-rwxr-xr-x   1 makmorit  staff  2097456  5 14 12:40 test
MacBookPro-makmorit-jp:test makmorit$
```

作成された`test`を実行し、`hello, world`と表示されれば成功です。

```
MacBookPro-makmorit-jp:test makmorit$ ./test
hello, world
MacBookPro-makmorit-jp:test makmorit$
```

以上でGO環境の導入は完了です。

## リポジトリーのクローン作成

以下のURLのリポジトリーのクローンを作成します。<br>
https://github.com/duo-labs/webauthn.git

下図はGitHubデスクトップでチェックアウトしたところです。

<img src="assets01/0009.png" width="700">

チェックアウトしたコードは、GOディレクトリー配下のsrcディレクトリー内に配置するようにします。<br>
配置イメージは下記のようになります。

<img src="assets01/0012.png" width="500">

## 依存ライブラリーの取得

ソースコードディレクトリー配下で`go get .`コマンドを実行すると、自動的に依存ライブラリーのダウンロードが行われます。

```
MacBookPro-makmorit-jp:webauthn makmorit$ cd /Users/makmorit/go/src/github.com/webauthn
MacBookPro-makmorit-jp:webauthn makmorit$ pwd
/Users/makmorit/go/src/github.com/webauthn
MacBookPro-makmorit-jp:webauthn makmorit$ go get .
MacBookPro-makmorit-jp:webauthn makmorit$
```

コマンド実行中に、別段のメッセージが表示されなければ、依存ライブラリーの取得は完了になります。

GOディレクトリー配下のbinディレクトリー内を確認し、`webauthn`という実行可能ファイルが生成されていることを確認します。

```
MacBookPro-makmorit-jp:bin makmorit$ pwd
/Users/makmorit/go/bin
MacBookPro-makmorit-jp:bin makmorit$ ls -al
total 45472
drwxr-xr-x  3 makmorit  staff       102  5 14 14:46 .
drwxr-xr-x  5 makmorit  staff       170  5 14 14:46 ..
-rwxr-xr-x  1 makmorit  staff  23279472  5 14 14:46 webauthn
MacBookPro-makmorit-jp:bin makmorit$
```

## 実行

まずは`config.template.json`というファイルをコピーして`config.json`を作成します。

```
MacBookPro-makmorit-jp:webauthn makmorit$ cp config.template.json config.json
MacBookPro-makmorit-jp:webauthn makmorit$ ls -al *.json
-rw-r--r--  1 makmorit  staff  378  5 14 14:31 config.json
-rw-r--r--  1 makmorit  staff  378  5 14 09:58 config.template.json
MacBookPro-makmorit-jp:webauthn makmorit$
```

内容を確認し、下記のように修正します。

```
{
	"db_name" : "sqlite3",
	"db_path" : "webauthn.db",
	"migrations_prefix" : "db/db_",
	"has_proxy": false,
	"host_address": "127.0.0.1",
	"host_port": ":9005"
}
```

次に、binディレクトリー内の`webauthn`という実行可能ファイルを、`config.json`と同じディレクトリー（`src/webauthn`ディレクトリー）にコピーします。

下記のように実行します。

```
MacBookPro-makmorit-jp:webauthn makmorit$ pwd
/Users/makmorit/go/src/webauthn
MacBookPro-makmorit-jp:webauthn makmorit$ ./webauthn
Config: {DBName:sqlite3 DBPath:webauthn.db MigrationsPath:db/db_sqlite3 HostAddress:localhost HostPort::9005 HasProxy:true}
```

実行がスタートします。

## 動作確認

Firefox 60を起動し、`http://127.0.0.1:9005`を実行してみます。<br>
デモサイトとまったくおなじインデックスページが表示されます。

<img src="assets01/0013.png" width="500">

あとは[こちらの手順書](WEBAUTHN_FF60.md)に従って操作します。<br>
（「デモサイトとYubikey NEOを使った確認」をご参照）

## 採取したログ

ご参考までに、以下に掲載させていただきます。

### ユーザー登録（Register）時のサーバーログ

```
Adding new Session Data
Decoded Client Data: &{RawClientData:{"challenge":"v6D8vaoRfwY4dkmZEKznRQ","clientExtensions":{},"hashAlgorithm":"SHA-256","origin":"http://127.0.0.1:9005","type":"webauthn.create"} Challenge:v6D8vaoRfwY4dkmZEKznRQ HashAlgorithm:SHA-256 Origin:http://127.0.0.1:9005 ActionType:webauthn.create}
Auth Data: &{Flags:[48 49 48 48 48 48 48 49] Counter:[0 0 0 0 0] RPIDHash:12ca17b49af2289436f303e0166030a21e525d266e209267433801a8fd4071a0 AAGUID:[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] CredID:[146 199 20 102 139 156 48 2 124 180 52 223 66 148 71 115 96 67 131 85 128 25 146 148 80 9 181 30 234 155 115 61 104 210 235 219 88 43 148 99 39 13 107 70 166 117 211 185 161 72 232 31 43 23 32 212 106 98 37 38 222 81 72 229] PubKey:{Model:{ID:0 CreatedAt:0001-01-01 00:00:00 +0000 UTC UpdatedAt:0001-01-01 00:00:00 +0000 UTC DeletedAt:<nil>} _struct:false KeyType:2 Type:-7 XCoord:[129 133 252 212 40 155 254 58 205 22 151 183 178 12 119 11 37 164 249 91 39 132 27 110 155 4 1 167 16 98 22 220] YCoord:[148 197 241 2 25 133 127 23 216 85 165 192 221 2 10 224 51 10 84 124 61 102 156 191 220 88 113 36 143 131 136 227] Curve:1 CredentialID:0} Format:none AttStatement:{Certificate:<nil> Signature:[]}}
Hash Alg: SHA-256
Client data hash is dca6243ecae80cd7966dba2e4e4ed1bd128621d4b1afbd00651b84921b1416f3
Creating Credential
{Model:{ID:1 CreatedAt:2018-05-16 04:24:37.443256696 +0000 UTC UpdatedAt:2018-05-16 04:24:37.443256696 +0000 UTC DeletedAt:<nil>} Counter:[0 0 0 0 0] RelyingParty:{ID:127.0.0.1 DisplayName:Acme, Inc Icon:lol.catpics.png Users:[]} RelyingPartyID:127.0.0.1 User:{Model:{ID:3 CreatedAt:2018-05-16 04:21:46.000113758 +0000 UTC UpdatedAt:2018-05-16 04:24:37.443067412 +0000 UTC DeletedAt:<nil>} Name:makmorittest@example.com DisplayName:makmorittest Icon: Credentials:[] RelyingParties:[]} UserID:3 Type:public-key Format:none Flags:[48 49 48 48 48 48 48 49] CredID:kscUZoucMAJ8tDTfQpRHc2BDg1WAGZKUUAm1Huqbcz1o0uvbWCuUYycNa0amddO5oUjoHysXINRqYiUm3lFI5Q PublicKey:{Model:{ID:1 CreatedAt:2018-05-16 04:24:37.443381293 +0000 UTC UpdatedAt:2018-05-16 04:24:37.443381293 +0000 UTC DeletedAt:<nil>} _struct:false KeyType:2 Type:-7 XCoord:[129 133 252 212 40 155 254 58 205 22 151 183 178 12 119 11 37 164 249 91 39 132 27 110 155 4 1 167 16 98 22 220] YCoord:[148 197 241 2 25 133 127 23 216 85 165 192 221 2 10 224 51 10 84 124 61 102 156 191 220 88 113 36 143 131 136 227] Curve:1 CredentialID:1}}
```


### 認証（Login）時のサーバーログ

```
Adding new Session Data
Decoded Client Data: &{RawClientData:{"challenge":"FnI_-3iGbCGmVpZwEe5q9Q","clientExtensions":{},"hashAlgorithm":"SHA-256","origin":"http://127.0.0.1:9005","type":"webauthn.get"} Challenge:FnI_-3iGbCGmVpZwEe5q9Q HashAlgorithm:SHA-256 Origin:http://127.0.0.1:9005 ActionType:webauthn.get}
Auth Data: &{Flags:1 Counter:[0 0 0 23] RawAssertionData:[18 202 23 180 154 242 40 148 54 243 3 224 22 96 48 162 30 82 93 38 110 32 146 103 67 56 1 168 253 64 113 160 1 0 0 0 23] RPIDHash:12ca17b49af2289436f303e0166030a21e525d266e209267433801a8fd4071a0 Signature:[48 69 2 32 3 41 245 87 244 244 90 81 210 134 232 137 193 189 157 56 70 224 182 176 126 137 3 149 58 108 97 23 25 219 43 245 2 33 0 203 124 209 130 8 67 60 110 147 42 253 237 218 154 198 98 252 200 75 41 195 158 137 230 95 40 255 108 226 109 87 241]}
Client data hash is 8ae5995d5c2383e8382ba44ebff1a71b445e52d5e79d6323d363cfc348302ab6
```

## Relying Parties（RP）について

FIDO 2.0では認証を行う主体を「Relying Parties（RP）」と称しています。<br>
これは署名済みサーバー証明書のコモンネームに対応しています。

### 認証処理におけるRP

IPアドレスが同じホストを指定して、ブラウザーからWebAuthnを実行時、RPとして指定したホストと異なる場合、FIDO 2.0の認証処理ができないという仕様になっております。

また、RPとしてIPアドレスを指定した場合、そのIPアドレスと同じホスト名称を指定しても、違うRPとみなされ認証処理が失敗します。

技術的には、サーバー側でRPとして指定したホスト名から得たハッシュ値と、ブラウザー側で指定したホスト名から得たハッシュ値を比較し、同じRPかどうかを判定しているための挙動のようです。

### ローカルテストサーバーでのRP指定方法

本件ローカルテストサーバーでは、このRPをソースコードにハードコードせず、データベース（sqlite3）のテーブルで管理しています。<br>
デフォルトは`127.0.0.1`（localhostのIPアドレス）と設定されています。

このため、ローカルテストサーバーをデフォルト設定のまま起動して、ブラウザーから`http://127.0.0.1:9005`以外のアドレスでアクセスすると、エラーになってしまいます。

（下記は`http://localhost:9005/`でアクセスした例です。サーバーはRPに`localhost`を指定し起動しています）
```
MacBookPro-makmorit-jp:webauthn makmorit$ ./webauthn
Config: {DBName:sqlite3 DBPath:webauthn.db MigrationsPath:db/db_sqlite3 HostAddress:localhost HostPort::9005 HasProxy:false}
No RP found for host  localhost
Request: &{Method:GET URL:/makeCredential/makmorittest@example.com Proto:HTTP/1.1 ProtoMajor:1 ProtoMinor:1 Header:map[X-Requested-With:[XMLHttpRequest] Cookie:[registration-session=MTUyNjUyMTYwMXxEdi1CQkFFQ180SUFBUkFCRUFBQUl2LUNBQUVHYzNSeWFXNW5EQXdBQ25ObGMzTnBiMjVmYVdRRWRXbHVkQVlDQUFjPXxxXFmOgGm1mANaCs50Rq8MnDjTtddt9fwErXvvMNqAIQ==] Connection:[keep-alive] User-Agent:[Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:60.0) Gecko/20100101 Firefox/60.0] Accept-Language:[ja,en-US;q=0.7,en;q=0.3] Accept-Encoding:[gzip, deflate] Referer:[http://localhost:9005/] Accept:[application/json, text/javascript, */*; q=0.01]] Body:{} GetBody:<nil> ContentLength:0 TransferEncoding:[] Close:false Host:localhost:9005 Form:map[] PostForm:map[] MultipartForm:<nil> Trailer:map[] RemoteAddr:127.0.0.1:50175 RequestURI:/makeCredential/makmorittest@example.com TLS:<nil> Cancel:<nil> Response:<nil> ctx:0xc4206171a0}
```

ホスト名`localhost`をRPに追加すれば、上記のようなエラーは発生しなくなります。<br>
ただし、RPのレコードをデータベース・テーブルに追加するユーティリティーはないので、以下の手順により、直接sqlite3コマンドを実行してレコードを追加します。

```
MacBookPro-makmorit-jp:webauthn makmorit$ sqlite3 webauthn.db
SQLite version 3.16.0 2016-11-04 19:09:39
Enter ".help" for usage hints.
sqlite> insert into relying_parties values ('localhost', 'Acme, Inc', 'lol.catpics.png');
sqlite> select * from relying_parties;
127.0.0.1|Acme, Inc|lol.catpics.png
localhost|Acme, Inc|lol.catpics.png
sqlite> .exit
MacBookPro-makmorit-jp:webauthn makmorit$
```

RPレコードを追加したら、サーバーを再度起動します。<br>
ブラウザーから`http://localhost:9005`を指定して実行すると、今度はサーバー側の処理が成功します。

```
MacBookPro-makmorit-jp:webauthn makmorit$ ./webauthn
Config: {DBName:sqlite3 DBPath:webauthn.db MigrationsPath:db/db_sqlite3 HostAddress:localhost HostPort::9005 HasProxy:false}
Adding new Session Data
Decoded Client Data: &{RawClientData:{"challenge":"FWkhTuyQ1SKRMGkEJ849Fg","clientExtensions":{},"hashAlgorithm":"SHA-256","origin":"http://localhost:9005","type":"webauthn.create"} Challenge:FWkhTuyQ1SKRMGkEJ849Fg HashAlgorithm:SHA-256 Origin:http://localhost:9005 ActionType:webauthn.create}
Auth Data: &{Flags:[48 49 48 48 48 48 48 49] Counter:[0 0 0 0 0] RPIDHash:49960de5880e8c687434170f6476605b8fe4aeb9a28632c7995cf3ba831d9763 AAGUID:[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0] CredID:[99 144 251 223 63 224 255 115 207 94 197 248 204 112 186 100 139 188 163 58 241 180 151 232 96 132 22 177 73 56 239 72 156 30 12 153 201 128 209 155 185 14 93 250 19 152 44 207 104 103 111 55 192 131 64 25 32 175 116 121 52 225 63 149] PubKey:{Model:{ID:0 CreatedAt:0001-01-01 00:00:00 +0000 UTC UpdatedAt:0001-01-01 00:00:00 +0000 UTC DeletedAt:<nil>} _struct:false KeyType:2 Type:-7 XCoord:[188 166 125 96 159 218 170 228 140 156 134 187 159 16 241 248 181 174 192 15 99 2 144 61 234 150 230 47 87 73 59 155] YCoord:[25 107 171 58 97 246 17 196 169 199 55 34 96 126 7 140 70 187 133 63 246 56 222 194 38 24 15 45 105 94 128 25] Curve:1 CredentialID:0} Format:none AttStatement:{Certificate:<nil> Signature:[]}}
Hash Alg: SHA-256
Client data hash is 575e0e1dc140b26ed6fd949ae8da7c51ad41a53b530754a40aa1535930f73525
Creating Credential
{Model:{ID:5 CreatedAt:2018-05-17 01:46:43.311633411 +0000 UTC UpdatedAt:2018-05-17 01:46:43.311633411 +0000 UTC DeletedAt:<nil>} Counter:[0 0 0 0 0] RelyingParty:{ID:localhost DisplayName:Acme, Inc Icon:lol.catpics.png Users:[]} RelyingPartyID:localhost User:{Model:{ID:4 CreatedAt:2018-05-17 01:36:23.497469952 +0000 UTC UpdatedAt:2018-05-17 01:46:43.311505361 +0000 UTC DeletedAt:<nil>} Name:makmorit@example.com DisplayName:makmorit Icon: Credentials:[] RelyingParties:[]} UserID:4 Type:public-key Format:none Flags:[48 49 48 48 48 48 48 49] CredID:Y5D73z_g_3PPXsX4zHC6ZIu8ozrxtJfoYIQWsUk470icHgyZyYDRm7kOXfoTmCzPaGdvN8CDQBkgr3R5NOE_lQ PublicKey:{Model:{ID:5 CreatedAt:2018-05-17 01:46:43.311757299 +0000 UTC UpdatedAt:2018-05-17 01:46:43.311757299 +0000 UTC DeletedAt:<nil>} _struct:false KeyType:2 Type:-7 XCoord:[188 166 125 96 159 218 170 228 140 156 134 187 159 16 241 248 181 174 192 15 99 2 144 61 234 150 230 47 87 73 59 155] YCoord:[25 107 171 58 97 246 17 196 169 199 55 34 96 126 7 140 70 187 133 63 246 56 222 194 38 24 15 45 105 94 128 25] Curve:1 CredentialID:5}}
```
