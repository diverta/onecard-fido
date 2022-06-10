# OATH管理コマンドについての調査

最終更新日：2022/5/30

OATH管理コマンドについての調査手順・結果を掲載しております。

## 調査環境

適宜セットアップしたPython3仮想環境下に、[Yubikey Manager](https://github.com/Yubico/yubikey-manager)をインストールし、コマンドを実行することにより調査を進めます。

#### 使用コマンド

[Yubikey Manager](https://github.com/Yubico/yubikey-manager)に付属するCLIツール（`ykman`というPythonツール）を使用します。

#### 使用方法

本件の調査環境では、以下のようにして使用します。

```
# 適宜作成したPython環境を立ち上げます。
cd ${HOME}/GitHub/yubikey-manager
source .venv/bin/activate

# CCIDインターフェースのAPDU内容がデバッグできないYubikey NEOの代わりに、
# nRF5340 DKに接続し、ykmanを起動します。
ykman -r "Diverta Inc. Secure Dongle" oath info
```

以下は動作例になります。<br>
最終更新日現在、下記のようにエラーが表示されますが、これはOATH関連の処理が、nRF5340に未実装であるためになります。

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath info
Error: The functionality required for this command is not enabled or not available on this YubiKey.
(.venv) bash-3.2$
```

## 解析結果

以下はコマンドの解析結果になります。

#### `info`
OATHアプレットのSELECTが実行されます。<br>
SELECTのレスポンスは、Yubico社の独自仕様になっているようです。<br>
（「[YKOATH Protocol Specification](https://developers.yubico.com/OATH/YKOATH_Protocol.html)」ご参照）

<b>`ykman`によるコマンド実行例</b><br>
認証器のOATHバージョン等の情報が表示されます。

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath info
OATH version: 1.0.0
Password protection: disabled
(.venv) bash-3.2$
```

<b>nRF5340アプリケーションからのデバッグ出力</b>

```
[00:00:06.478,179] <dbg> ccid_oath.ccid_oath_apdu_process: APDU recv: CLA INS P1 P2(00 a4 04 00) Lc(7) Le(256)
[00:00:06.478,210] <dbg> ccid_oath: APDU data
                                    a0 00 00 05 27 21 01                             |....'!.          
[00:00:06.478,210] <dbg> ccid_oath.ccid_oath_apdu_process: APDU send: SW(9000)
[00:00:06.478,210] <dbg> ccid_oath: APDU data
                                    79 03 01 00 00 71 08 95  b8 2a d4 51 04 e4 04    |y....q.. .*.Q...
```

#### `list`
OATHアプレットの独自INSコード`0xa1`が実行されます。<br>
こちらもレスポンスは、Yubico社の独自仕様になっているようです。<br>
（「[YKOATH Protocol Specification](https://developers.yubico.com/OATH/YKOATH_Protocol.html)」ご参照）

<b>`ykman`によるコマンド実行例</b><br>
認証器に登録されているOATHアカウント（２件）が取得されます。

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath accounts list
Example:alice@google.com
Example:bob@google.co
(.venv) bash-3.2$
```

<b>nRF5340アプリケーションからのデバッグ出力</b> [注]

```
[00:02:26.414,337] <dbg> ccid_oath.ccid_oath_apdu_process: APDU recv: CLA INS P1 P2(00 a1 00 00) Lc(0) Le(256)
[00:02:26.414,337] <dbg> ccid_oath: APDU data
[00:02:26.414,367] <dbg> ccid_oath.ccid_oath_apdu_process: APDU send: SW(9000)
[00:02:26.414,367] <dbg> ccid_oath: APDU data
                                    72 19 21 45 78 61 6d 70  6c 65 3a 61 6c 69 63 65 |r.!Examp le:alice
                                    40 67 6f 6f 67 6c 65 2e  63 6f 6d 72 16 21 45 78 |@google. comr.!Ex
                                    61 6d 70 6c 65 3a 62 6f  62 40 67 6f 6f 67 6c 65 |ample:bo b@google
                                    2e 63 6f                                         |.co  
```

[注] 最終更新日現在、nRF5340アプリケーションのCCIDインターフェースが、６４バイト以上のデータを送信できない不具合があるようです。この件につきましては、後日修正対応を行う予定です。

#### `add`
OATHアプレットの`INS_PUT`（`0x01`）が実行されます。<br>
レスポンスはステータスワードのみのようです。

<b>`ykman`によるコマンド実行例</b><br>
下記例では、認証器内に既に登録済みのアカウント`Example:alice@google.com`について、SECRETを上書き登録しようとしています。[注]

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath accounts uri "otpauth://totp/Example:alice@google.com?secret=JBSWY3DPEHPK3PXP&issuer=Example"
An account called alice@google.com already exists on this YubiKey. Do you want to overwrite it? [y/N]: y
(.venv) bash-3.2$
```

<b>nRF5340アプリケーションからのデバッグ出力</b>

```
[00:00:11.540,710] <dbg> ccid_oath.ccid_oath_apdu_process: APDU recv: CLA INS P1 P2(00 01 00 00) Lc(44) Le(256)
[00:00:11.540,740] <dbg> ccid_oath: APDU data
                                    71 18 45 78 61 6d 70 6c  65 3a 61 6c 69 63 65 40 |q.Exampl e:alice@
                                    67 6f 6f 67 6c 65 2e 63  6f 6d 73 10 21 06 48 65 |google.c oms.!.He
                                    6c 6c 6f 21 de ad be ef  00 00 00 00             |llo!.... ....    
[00:00:11.540,740] <dbg> ccid_oath.ccid_oath_apdu_process: APDU send: SW(9000)
```

[注] コマンドで指定したSECRET（`JBSWY3DPEHPK3PXP`）をBase32 Decodeすると、コマンド内でHEX（`48 65 6c 6c 6f 21 de ad be ef 00 00 00 00`）に変換され、nRF5340アプリケーションに送信されます。

#### `delete`
OATHアプレットの`INS_DELETE`（`0x02`）が実行されます。<br>
レスポンスはステータスワードのみのようです。

<b>`ykman`によるコマンド実行例</b><br>
下記例では、認証器内に既に登録済みのアカウント`Example:bob@google.co`について、アカウントを削除しようとしています。

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath accounts delete Example:bob@google.co
Delete account: Example:bob@google.co ? [y/N]: y
Deleted Example:bob@google.co.
(.venv) bash-3.2$
```

<b>nRF5340アプリケーションからのデバッグ出力</b>

```
[00:00:35.389,739] <dbg> ccid_oath.ccid_oath_apdu_process: APDU recv: CLA INS P1 P2(00 02 00 00) Lc(23) Le(256)
[00:00:35.389,770] <dbg> ccid_oath: APDU data
                                    71 15 45 78 61 6d 70 6c  65 3a 62 6f 62 40 67 6f |q.Exampl e:bob@go
                                    6f 67 6c 65 2e 63 6f                             |ogle.co          
[00:00:35.389,770] <dbg> ccid_oath.ccid_oath_apdu_process: APDU send: SW(9000)
```

#### `calculate`
OATHアプレットの独自INSコード`0xa4`が実行されます。[注1]<br>
レスポンスは計算されたパスワードになります。

<b>`ykman`によるコマンド実行例</b><br>
下記例では、認証器内に既に登録済みのアカウント`Example:alice@google.com`について、TOTPパスワードを計算しようとしています。<br>
レスポンスは`373492`という６桁のパスワードです。

```
(.venv) bash-3.2$ ykman -r "Diverta Inc. Secure Dongle" oath accounts code Example:alice@google.com
Example:alice@google.com  373492
(.venv) bash-3.2$
```

<b>nRF5340アプリケーションからのデバッグ出力</b>

まず、認証器にchallengeパラメーター（`00 00 00 00 03 49 36 f3`）が渡されます。<br>
これに基づいて認証器側がTOTPを計算して戻します。

概ね下記ロジックになっているようです。
- `ykman`側で、現在時刻からカウンターを計算（カウンター = UNIX時間 / `30`秒）し、challengeパラメーター生成
- challengeパラメーターと、認証器内で保持しているSECRETから、20バイトのHMAC-SHA1ハッシュ値を計算
- 前項ハッシュ値から、所定のロジックにより31ビットを取得し数値化-->４バイトのTOTPを生成

`ykman`側では、戻ってきたTOTP（`31 32 33 34`）を数値化（６桁の数字でフォーマット[注2]）して表示させています。

```
[00:00:10.185,852] <dbg> ccid_oath.ccid_oath_apdu_process: APDU recv: CLA INS P1 P2(00 a4 00 01) Lc(10) Le(256)
[00:00:10.185,852] <dbg> ccid_oath: APDU data
                                    74 08 00 00 00 00 03 49  36 f3                   |t......I 6.      
[00:00:10.185,852] <dbg> ccid_oath.ccid_oath_apdu_process: APDU send: SW(9000)
[00:00:10.185,882] <dbg> ccid_oath: APDU data
                                    71 18 45 78 61 6d 70 6c  65 3a 61 6c 69 63 65 40 |q.Exampl e:alice@
                                    67 6f 6f 67 6c 65 2e 63  6f 6d 76 05 06 31 32 33 |google.c omv..123
                                    34                                               |4
```

[注1] 正確には`INS=0xa4, P1=0x00, P2=0x01`となっています。<br>
[注2] 厳密には、戻ってきたTOTP（４バイト）を10進数変換-->下６桁を取得しているようです。
