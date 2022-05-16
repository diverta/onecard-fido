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
