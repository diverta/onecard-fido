# Yubico PIV Tool (command line) 導入手順

Yubico PIV Tool (command line) を、macOS環境に導入する手順を掲載します。

## 作業手順

### CCIDドライバーのインストール
まず事前に、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールしてください。

詳細な手順につきましては、手順書「<b>[CCIDドライバーインストール手順](../CCID/INSTALLPRG.md)</b>」をご参照願います。

CCIDドライバーのインストールが完了したら、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をPCのUSBポートに装着すると、MDBT50Q DongleがmacOSにより、スマートカード・デバイスとして認識されるようになります。

### 実行可能ファイルの取得

Webブラウザーから、サイト「[https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/](https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/)」を開きます。<br>
下記の様な画面に遷移します。

<img src="assets01/0006.jpg" width="500">

下にスクロールすると「Yubico PIV Tool (command line)」のリンクが参照できます。<br>
今回は、macOS環境にインストールするので「macOS download」のリンクをクリックします。

<img src="assets01/0007.jpg" width="500">

ダウンロードが完了したら、ダウンロードフォルダーを開きます。

<img src="assets01/0008.jpg" width="400">

ダウンロード・フォルダーにある「`yubico-piv-tool-2.0.0-mac.zip`」というファイルを展開します。

<img src="assets01/0009.jpg" width="400">

展開されたフォルダー「`yubico-piv-tool-2.0.0-mac`」は、分かりやすいところに適宜移動します。<br>
下図は、ホームディレクトリー配下の「`${HOME}/opt/yubico-piv-tool-2.0.0-mac`」に移動した例になります。

<img src="assets01/0010.jpg" width="400">

実行可能ファイルは、前述のフォルダー配下の「/bin/yubico-piv-tool」というファイルになります。<br>
こちらのファイルは実行権限がないため、ターミナルで以下のコマンドを実行し、実行権限を付与します。

```
chmod +x yubico-piv-tool
```

実行例は以下になります。

```
MacBookPro-makmorit-jp:~ makmorit$ cd ${HOME}/opt/yubico-piv-tool-2.0.0-mac/bin/
MacBookPro-makmorit-jp:bin makmorit$
MacBookPro-makmorit-jp:bin makmorit$ ls -al
total 240
drwxr-xr-x@ 3 makmorit  staff     102  1 30  2020 .
drwxr-xr-x@ 8 makmorit  staff     272  9 22 14:58 ..
-rw-r--r--@ 1 makmorit  staff  119632  1 30  2020 yubico-piv-tool
MacBookPro-makmorit-jp:bin makmorit$ chmod +x yubico-piv-tool
MacBookPro-makmorit-jp:bin makmorit$ ls -al
total 240
drwxr-xr-x@ 3 makmorit  staff     102  1 30  2020 .
drwxr-xr-x@ 8 makmorit  staff     272  9 22 14:58 ..
-rwxr-xr-x@ 1 makmorit  staff  119632  1 30  2020 yubico-piv-tool
MacBookPro-makmorit-jp:bin makmorit$
```

以上で、実行可能ファイルの準備ができました。

### 動作確認

ターミナルから、実行可能ファイル「`${HOME}/opt/yubico-piv-tool-2.0.0-mac/bin/yubico-piv-tool`」を使用し、以下のコマンドを実行します。

#### デバイスの名称を調べる

以下のコマンドを実行し、表示された結果から、デバイスの名称を調べます。

```
yubico-piv-tool -v -a status
```

以下は実行例になります。<br>
実行結果から、PCに装着されたデバイスが「`Diverta Inc. Secure Dongle`」という名称であることが分かります。

```
MacBookPro-makmorit-jp:bin makmorit$ pwd
/Users/makmorit/opt/yubico-piv-tool-2.0.0-mac/bin
MacBookPro-makmorit-jp:bin makmorit$ ./yubico-piv-tool -v -a status
Skipping reader 'Diverta Inc. Secure Dongle' since it doesn't match 'Yubikey'.
No usable reader found matching 'Yubikey'.
Failed to connect to yubikey.
Try removing and reconnecting the device.MacBookPro-makmorit-jp:bin makmorit$
```

#### デバイスの情報を取得する

前項で調べたデバイス名称「`Diverta Inc. Secure Dongle`」を、引数に指定して、以下のコマンドを実行します。

```
yubico-piv-tool --reader="Diverta Inc. Secure Dongle" -v -a status
```

以下は実行例になります。<br>
デバイスに接続され、CCIDインターフェース経由でPIVに関する情報が取得されます。<br>
情報取得が完了すると、自動的にデバイスから切断されます。

```
MacBookPro-makmorit-jp:bin makmorit$ ./yubico-piv-tool --reader="Diverta Inc. Secure Dongle" -v -a status
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'status' does not need authentication.
Now processing for action 'status'.
Version:	5.0.0
Serial Number:	0
CHUID:	3019d4e739da739ced39ce739d836858210842108421c84210c3eb34104f223f9d9d8fa33cbdcc0f6640e7ab89350832303330303130313e00fe00
CCC:	f015a000000116ff02f7fd97076c87749d478308461959f10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00
Slot 9a:
	Algorithm:	ECCP256
	Subject DN:	CN=pivauth.divert.co.jp
	Issuer DN:	CN=pivauth.divert.co.jp
	Fingerprint:	4092a0590ccb88ec34b6fcf292502e1182ed062f45e8948feb070a702ba658e6
	Not Before:	Sep 22 03:53:14 2020 GMT
	Not After:	Sep 22 03:53:14 2021 GMT
Slot 9c:
	Algorithm:	ECCP256
	Subject DN:	CN=digsign.divert.co.jp
	Issuer DN:	CN=digsign.divert.co.jp
	Fingerprint:	5868d46e1483de88b23b87d354ed6896d80981130866c915d86205318a8f9587
	Not Before:	Sep 22 03:53:22 2020 GMT
	Not After:	Sep 22 03:53:22 2021 GMT
Slot 9d:
	Algorithm:	ECCP256
	Subject DN:	CN=keymgmt.divert.co.jp
	Issuer DN:	CN=keymgmt.divert.co.jp
	Fingerprint:	2ad04972e4047b2545ba2fc71ffadb70999bac374849145b35240d613a710a61
	Not Before:	Sep 22 03:53:29 2020 GMT
	Not After:	Sep 22 03:53:29 2021 GMT
PIN tries left:	3
Disconnect card #0.
MacBookPro-makmorit-jp:bin makmorit$
```

以上で、Yubico PIV Tool (command line) 導入は終了となります。
