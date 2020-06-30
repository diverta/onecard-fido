# Yubico PIV Tool (command line) 導入手順

Yubico PIV Tool (command line) を、Windows 10環境に導入する手順を掲載します。

## 作業手順

### 実行可能ファイルの取得

Webブラウザーから、サイト「[https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/](https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/)」を開きます。<br>
下記の様な画面に遷移します。

<img src="assets01/0001.jpg" width="500">

下にスクロールすると「Yubico PIV Tool (command line)」のリンクが参照できます。<br>
今回は、Windows 10環境（64bit）にインストールするので「Windows for 64-bit systems download」のリンクをクリックします。

<img src="assets01/0002.jpg" width="500">

ダウンロードが完了したら、ダウンロードフォルダーを開きます。

<img src="assets01/0003.jpg" width="500">

ダウンロード・フォルダーにある「`yubico-piv-tool-2.0.0-win64.zip`」というファイルを展開します。

<img src="assets01/0004.jpg" width="400">

展開されたフォルダー「`yubico-piv-tool-2.0.0-win64`」は、分かりやすいところに適宜移動します。<br>
下図は、デスクトップに移動した例になります。

<img src="assets01/0005.jpg" width="300">

### 動作確認

Windowsのコマンドプロンプトから、実行可能ファイル「`yubico-piv-tool-2.0.0-win64\bin\yubico-piv-tool`」を使用し、以下のコマンドを実行します。

#### デバイスの名称を調べる

以下のコマンドを実行し、表示された結果から、デバイスの名称を調べます。

```
yubico-piv-tool -v --action=status
```

以下は実行例になります。<br>
実行結果から、PCに装着されたデバイスが「`Canokeys OpenPGP PIV OATH 0`」という名称であることが分かります。

```
yubico-piv-tool-2.0.0-win64\bin>yubico-piv-tool -v --action=status
Skipping reader 'Canokeys OpenPGP PIV OATH 0' since it doesn't match 'Yubikey'.
No usable reader found matching 'Yubikey'.
Failed to connect to yubikey.
Try removing and reconnecting the device.
```

#### デバイスの情報を取得する

前項で調べたデバイス名称「`Canokeys OpenPGP PIV OATH 0`」を、引数に指定して、以下のコマンドを実行します。

```
yubico-piv-tool -v --reader="Canokeys OpenPGP PIV OATH 0" --action=status
```

以下は実行例になります。<br>
デバイスに接続され、CCIDインターフェース経由でPIVに関する情報が取得されます。<br>
情報取得が完了すると、自動的にデバイスから切断されます。

```
yubico-piv-tool-2.0.0-win64\bin>yubico-piv-tool -v --reader="Canokeys OpenPGP PIV OATH 0" --action=status
Connect reader 'Canokeys OpenPGP PIV OATH 0' matching 'Canokeys OpenPGP PIV OATH 0'.
Action 'status' does not need authentication.
Now processing for action 'status'.
Version:        5.0.0
Serial Number:  1588739612
CHUID:  3019d4e739da739ced39ce739d836858210842108421c84210c3eb34104c8d536a86aa98a5ce20d53557776e58350832303330303130313e00fe00
CCC:    f015a000000116ff02d4bfab488d66fa69ae507ee5f8daf10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00
Slot 9a:
        Algorithm:      RSA2048
        Subject DN:     CN=pivauth.divert.co.jp
        Issuer DN:      CN=pivauth.divert.co.jp
        Fingerprint:    44d0ea3dca57c5c83f7615d7627e97935b236c64cc08b5fd08c35bd64a34cae3
        Not Before:     May 18 06:06:55 2020 GMT
        Not After:      May 18 06:06:55 2021 GMT
Slot 9c:
        Algorithm:      RSA2048
        Subject DN:     CN=digsign.divert.co.jp
        Issuer DN:      CN=digsign.divert.co.jp
        Fingerprint:    28b7be8ada3504e2cb22acb32ec53d0bd0ccd0baf444010f27b87eb05e95555a
        Not Before:     May 18 06:07:56 2020 GMT
        Not After:      May 18 06:07:56 2021 GMT
Slot 9d:
        Algorithm:      RSA2048
        Subject DN:     CN=keymgmt.divert.co.jp
        Issuer DN:      CN=keymgmt.divert.co.jp
        Fingerprint:    07b879e12ac0c82d31599684fc265b5ddbbc14f02477e0b2e3459f4fc72775fb
        Not Before:     May 20 06:15:37 2020 GMT
        Not After:      May 20 06:15:37 2021 GMT
PIN tries left: 3
Disconnect card #1588739612.
```

以上で、Yubico PIV Tool (command line) 導入は終了となります。
