# Yubico PIV Toolによる各種手順

Yubico PIV Tool (command line) を使用した各種手順を掲載します。<br>
本ページでは、以下が説明対象になります。
- PINのリセット
- PUKの変更（オプション）
- PIV機能のリセット

## 事前準備

Yubico PIV Tool (command line) を、PC環境上で使用できるようにするための手順は下記手順書をご参照願います。
- <b>[Yubico PIV Tool (command line) macOS版 導入手順](../CCID/PIVTOOLMACINST.md)</b>
- <b>[Yubico PIV Tool (command line) Windows版 導入手順](../CCID/PIVTOOLWININST.md)</b>

## 各種手順

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をPCのUSBポートに装着します。

ターミナル（コマンドプロンプト）から、Yubico PIV Tool (command line) の実行可能ファイル「`${HOME}/opt/yubico-piv-tool-2.0.0-mac/bin/yubico-piv-tool`」を使用し、以下のコマンドを実行します。

#### 接続確認

コマンド`list-readers`を実行し、PIVデバイスの一覧を画面表示します。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a list-readers
```

以下は実行例になります。<br>
MDBT50Q Dongleと接続できたことが、メッセージにより確認できます。

```
bash-3.2$ cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
bash-3.2$ pwd
/Users/makmorit/opt/yubico-piv-tool-2.0.0/bin
bash-3.2$
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a list-readers
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'list-readers' does not need authentication.
Now processing for action 'list-readers'.
Diverta Inc. Secure Dongle
Disconnect card #0.
bash-3.2$
```

#### PINのリセット

PIN番号を３回以上間違えて指定すると、PIVのPINポリシーにより、PIV番号による認証がブロックされます。<br>
このブロックを解除するためには、PUKという暗証番号を使用して認証し、PIN番号を再設定する必要があります。<br>
本ページでは、この設定を「PINのリセット」と称します。

コマンド`unblock-pin`を実行し、PINのリセットを行います。<br>
以下のコマンドを実行します。[注1]

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a unblock-pin -P 12345678
```

以下は実行例になります。<br>

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a unblock-pin -P 12345678
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'unblock-pin' does not need authentication.
Now processing for action 'unblock-pin'.
Enter new pin: [注2]
Verifying - Enter new pin: [注2]
Successfully unblocked the pin code.
Disconnect card #0.
bash-3.2$
```

[注1]`-P 12345678`は、デフォルトPUK番号を指定した例です。後述手順でPUK番号をデフォルトから変更した場合は、そのPUK番号を代わりに指定してください。<br>
[注2]`Enter new pin: `や`Verifying - Enter new pin: `のプロンプトに続いて、再設定したいPIN番号を入力します。入力文字列はエコーバックされません。

#### PUKの変更（オプション）

現状、PIV機能で使用するデフォルトのPUK番号は、`12345678` となっております。<br>
こちらを適宜、別のPUK番号に変更できます。

コマンド`change-puk`を実行し、PIV機能で使用するPUK番号を変更します。<br>
以下のコマンドを実行します。[注1]

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a change-puk -P 12345678
```

以下は実行例になります。<br>

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a change-puk -P 12345678
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'change-puk' does not need authentication.
Now processing for action 'change-puk'.
Enter new puk: [注2]
Verifying - Enter new puk: [注2]
Successfully changed the puk code.
Disconnect card #0.
bash-3.2$
```

[注1]`-P 12345678`は、デフォルトPUK番号を指定した例です。本手順でPUK番号をデフォルトから変更した場合は、そのPUK番号を代わりに指定してください。<br>
[注2]`Enter new puk: `や`Verifying - Enter new puk: `のプロンプトに続いて、新しいPUK番号を入力します。入力文字列はエコーバックされません。

#### PIV機能のリセット

PUK番号を３回以上間違えて指定すると、PIVのPINポリシーにより、PUK番号による認証がブロックされます。<br>
結果として、前述のPINリセットを実行することができなくなります。

万が一、PINもすでにブロックされている場合、PINリセット（PIN番号の再設定）が実行できないため、PINを使用するPIV機能が全面的に使用不可能となってしまいます。<br>
PIV機能の利用を再開できるようにするため、PIVでは「PIVアプリケーションリセット」という機能（＝PIV機能のリセット）が用意されています。

<b>【ご注意】<br>
PIV機能のリセットを実行すると、MDBT50Q Dongle内に導入した秘密鍵・証明書がすべて消去されます。<br>
「[Yubico PIV Toolによる初期データ導入手順](YKPIVUSAGE.md)」により、再度初期データを導入して下さい。</b>

コマンド`reset`を実行し、PIV機能のリセットを実行します。<br>
以下のコマンドを実行します。

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a reset
```

以下は実行例になります。<br>

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a reset
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'reset' does not need authentication.
Now processing for action 'reset'.
Successfully reset the application.
Disconnect card #0.
bash-3.2$
```
