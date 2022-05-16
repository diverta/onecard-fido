# OATH管理コマンドについての調査

最終更新日：2022/5/16

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

<b>`ykman`によるコマンド実行例</b>

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
