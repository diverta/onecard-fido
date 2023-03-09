# CCIDドライバー修正ビルド手順

最終更新日：2023/3/9

macOSにプレインストールされているCCIDドライバーを修正ビルドする手順について掲載しています。

#### 経緯・目的

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)はUSB-IF（[USB Implementers Forum, Inc.](https://www.usb.org)）に参加・届出をしていない製造者のデバイスであるため、macOSでCCIDデバイスとして認識されません。<br>
このため、MDBT50Q Dongleは、macOS上でCCIDインターフェースを使用する機能（PIV、OpenPGP、OATH）を実行させることができません。

他方、MDBT50Q Dongleは仮の製造元ID「`0xF055`」が付与されています。<br>
（当然ですが、これはUSB-IFの管理対象ではありません）<br>
この製造元IDを、macOSにインストールされているCCIDドライバーで管理対象とさせるためには、macOSにプレインストールされているCCIDドライバーを、ソースコードから修正ビルドする必要があります。

本ドキュメントの手順によりCCIDドライバーを修正ビルドし、これをmacOSにインストールすることにより、MDBT50Q DongleをCCIDデバイスとしてmacOSに認識させることができるようになります。

#### 注意点

本手順書で使用するCCIDドライバーは、その更新のたびに追従作業が必要となります。<br>
適宜、[USB-IFの該当ページ](https://ccid.apdu.fr/files/)で、更新情報を確認する必要があります。

最終更新日現在のCCIDドライバーのバージョンは、[`1.5.2`](https://salsa.debian.org/rousseau/CCID/blob/master/README.md)となっております。

## 作業手順

Apple Silicon向け、Intel mac向けに、それぞれ別々のCCIDドライバーを作成するようにします。<br>
手順は両者とも同一になります。

#### パッケージを導入

ビルド作業のために必要となるパッケージを、`brew`などで事前にインストールしておきます。<br>
通常のmacOS環境では`libusb`、`pkg-config`が導入されていないため、追加インストールします。

以下のコマンドを実行します。

```
brew install libusb
brew install pkg-config
```

`libusb`は、`/opt/homebrew/Cellar/libusb/1.0.26/lib`に導入されます。

```
bash-3.2$ cd /opt/homebrew/Cellar/libusb/1.0.26/lib
bash-3.2$ ls -al
total 752
drwxr-xr-x   7 devmorit  admin     224  3  9 16:40 .
drwxr-xr-x  14 devmorit  admin     448  3  8 18:27 ..
-r--r--r--   1 devmorit  admin  177456  3  8 17:48 libusb-1.0.0.dylib
-r--r--r--   1 devmorit  admin  194408  4 10  2022 libusb-1.0.a
lrwxr-xr-x   1 devmorit  admin      18  4 10  2022 libusb-1.0.dylib -> libusb-1.0.0.dylib
drwxr-xr-x   3 devmorit  admin      96  3  8 17:48 pkgconfig
bash-3.2$
```

ビルド作業や、ビルド後のCCIDドライバー実行の際は、動的リンクが不要であるため、必ず`.dylib`ファイルをリネームしておくようにします。

```
bash-3.2$ cd /opt/homebrew/Cellar/libusb/1.0.26/lib
bash-3.2$ mv libusb-1.0.0.dylib __libusb-1.0.0.dylib
bash-3.2$ mv libusb-1.0.dylib __libusb-1.0.dylib
bash-3.2$
bash-3.2$ ls -al
total 752
drwxr-xr-x   7 devmorit  admin     224  3  9 16:41 .
drwxr-xr-x  14 devmorit  admin     448  3  8 18:27 ..
-r--r--r--   1 devmorit  admin  177456  3  8 17:48 __libusb-1.0.0.dylib
lrwxr-xr-x   1 devmorit  admin      18  4 10  2022 __libusb-1.0.dylib -> libusb-1.0.0.dylib
-r--r--r--   1 devmorit  admin  194408  4 10  2022 libusb-1.0.a
drwxr-xr-x   3 devmorit  admin      96  3  8 17:48 pkgconfig
bash-3.2$
```

#### ソースコードを取得

下記サイトからソースコード（`ccid-1.5.2.tar.bz2`）をダウンロードします。
- <b>[CCID free software driver](https://ccid.apdu.fr)</b>

サイト[`https://ccid.apdu.fr`](https://ccid.apdu.fr)を表示し、青い「Download」ボタンをクリックします。

<img src="assets03/0001.jpg" width="640">

遷移先ページにある「Latest version」のリンクをクリックします。

<img src="assets03/0002.jpg" width="640">

ダウンロードした「`ccid-1.5.2.tar.bz2`」を解凍します。

<img src="assets03/0003.jpg" width="480">

解凍したフォルダー`ccid-1.5.2`を、`${HOME}/opt/`配下に<b>移動します</b>。

```
bash-3.2$ ls -al ${HOME}/Downloads
total 1400
drwx------+  6 devmorit  staff     192  3  9 16:47 .
drwxr-x---+ 24 devmorit  staff     768  3  9 16:37 ..
drwxr-xr-x@ 31 devmorit  staff     992  2  1 00:28 ccid-1.5.2
-rw-r--r--@  1 devmorit  staff  705174  3  8 16:58 ccid-1.5.2.tar.bz2
bash-3.2$ mv ${HOME}/Downloads/ccid-1.5.2 ${HOME}/opt
bash-3.2$
bash-3.2$ ls -al ${HOME}/opt/
total 24
drwxr-xr-x   6 devmorit  staff   192  3  9 16:53 .
drwxr-x---+ 24 devmorit  staff   768  3  9 16:37 ..
drwxr-xr-x@ 31 devmorit  staff   992  2  1 00:28 ccid-1.5.2
drwxr-xr-x   8 devmorit  staff   256  3  8 15:14 openssl
drwxrwxr-x@ 24 devmorit  staff   768  2 23 17:37 tinycbor
bash-3.2$
```

#### メイクファイルの生成

ソースコードフォルダー`ccid-1.4.32`に移動したら、シェル`./MacOSX/configure`を実行し、メイクファイルを生成します。<br>
以下のコマンドを実行します。（実行例は<b>[こちら](assets03/ccid_make_arm64.log)</b>）

```
cd ${HOME}/opt/ccid-1.5.2/
./MacOSX/configure
```

#### サポートデバイスリストの修正内容

新たにmacOSでサポートさせたいデバイスとなる、MDBT50Q Dongleの製品ID（VID／PID）を、サポートデバイスリストに追記します。

MDBT50Q Dongleは、製造元ID＝`0xF055`、製品ID＝`0x0001`とします。<br>
この値を、`readers/supported_readers.txt`というファイルに追記します。<br>
記述フォーマットは下記の通りです。

```
# <製造者名>
<VID>:<PID>:<製造者名> <製品名>
```

下記は設定例になります。
- 製造者名: `Diverta Inc.`
- 製品名: `Secure Dongle`
- VID: `0xF055`
- PID: `0x0001`

```
# Diverta Inc.
0xF055:0x0001:Diverta Inc. Secure Dongle
```

#### サポートデバイスリストの修正（パッチ適用）
先述のサポートデバイスリスト`supported_readers.txt`を修正するためのパッチを用意しております。<br>
以下のコマンドを実行すると、エディター等を使用しなくても、サポートデバイスリストを修正することができます。

```
cd ${HOME}/opt/ccid-1.5.2/
patch readers/supported_readers.txt < ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/supported_readers.txt.patch
```

下記は実行例になります。

```
bash-3.2$ patch readers/supported_readers.txt < ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/supported_readers.txt.patch
patching file readers/supported_readers.txt
Hunk #1 succeeded at 940 with fuzz 2 (offset 148 lines).
bash-3.2$
bash-3.2$ cat readers/supported_readers.txt | grep Diverta
# Diverta Inc.
0xF055:0x0001:Diverta Inc. Secure Dongle
bash-3.2$
```

#### ビルドの実行

`make`コマンドを実行し、ビルドを実行します。

```
bash-3.2$ make
/Applications/Xcode.app/Contents/Developer/usr/bin/make  all-recursive
Making all in readers
make[2]: Nothing to be done for `all'.
Making all in examples
  CC       scardcontrol-scardcontrol.o
:
  CCLD     libccid.la
ld: warning: -undefined dynamic_lookup may not work with chained fixups
make[2]: Nothing to be done for `all-am'.
bash-3.2$
bash-3.2$ echo $?
0
bash-3.2$
```

以上でビルドは完了になります。

#### インストールの実行

動作確認を行うため、コマンド`sudo make install`を使用し、ビルドしたCCIDドライバーをシステム（macOS）にインストールします。

```
bash-3.2$ sudo make install
Password:
Making install in readers
:
Making install in src
./create_Info_plist.pl ./../readers/supported_readers.txt ./Info.plist.src --target=libccid.dylib --version=1.5.2  > Info.plist
/bin/sh /Users/devmorit/opt/ccid-1.5.2/install-sh -d "/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS/"
cp Info.plist "/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/"
cp .libs/libccid.dylib "/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS/libccid.dylib"
:
bash-3.2$
```

この作業をおこなっても、macOSにプレインストールされたCCIDドライバー`/usr/libexec/SmartCardServices`は、上書き修正されません。<br>
本件インストール実行では、`/usr/local/libexec/SmartCardServices`という場所にインストールされます。

インストールされたCCIDドライバーのパッケージファイルを直接参照し、新たに追加したサポート対象デバイスが、サポートデバイスリストに記載されていることを為念確認します。

```
bash-3.2$ cd /usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/
bash-3.2$ cat Info.plist | grep Diverta
		<string>Diverta Inc. Secure Dongle</string>
bash-3.2$
```

以上でインストールは完了になります。

#### 動作確認

PCを再起動し、インストールしたCCIDドライバーを有効化させます。<br>
その後、MDBT50Q Dongle（＝新たに追加されたサポート対象デバイス）を、PCのUSBポートに接続します。

macOSの「システム情報」アプリを実行し、認識・接続されるかどうかを確認します。<br>
下図のように、製品ID＝`0x0001`、製造元ID＝`0xF055`が表示されればOKです。

<img src="assets03/0004.jpg" width="500">

以上で動作確認は完了です。

## インストーラーの作成

他のmacOSを搭載したPCに、上記手順で作成したCCIDドライバーをインストールするためのインストーラー（再頒布用パッケージ）を作成します。

#### インストーラー作成用媒体の配置

`${HOME}/GitHub/onecard-fido/CCID/macOSDriver`に、インストーラーを作成するためのファイルを配置します。

まずは、このフォルダーの直下に`CCIDDriver.plist`というファイルを以下の内容で生成します。

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<array>
	<dict>
		<key>BundleHasStrictIdentifier</key>
		<true/>
		<key>BundleIsRelocatable</key>
		<false/>
		<key>BundleIsVersionChecked</key>
		<false/>
		<key>BundleOverwriteAction</key>
		<string>upgrade</string>
		<key>RootRelativeBundlePath</key>
		<string>ifd-ccid.bundle</string>
	</dict>
</array>
</plist>
```

次に、`bundle_xxx`というサブディレクトリーに、前述の手順で生成したドライバー`ifd-ccid.bundle`を、権限を変えずにコピーします。<br>

[Apple silicon向け]
```
cp -prv /usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64
```

[Intel mac向け]
```
cp -prv /usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/bundle_x86
```


以下は実行例になります。

```
bash-3.2$ cp -prv /usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle -> /Users/devmorit/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64/ifd-ccid.bundle
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents -> /Users/devmorit/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64/ifd-ccid.bundle/Contents
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS -> /Users/devmorit/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64/ifd-ccid.bundle/Contents/MacOS
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS/libccid.dylib -> /Users/devmorit/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64/ifd-ccid.bundle/Contents/MacOS/libccid.dylib
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/Info.plist -> /Users/devmorit/GitHub/onecard-fido/CCID/macOSDriver/bundle_arm64/ifd-ccid.bundle/Contents/Info.plist
bash-3.2$
```

#### インストーラーの作成

インストーラー作成のために必要なファイルが揃ったら、以下のコマンドを実行します。

[Apple silicon向け]
```
cd ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/
rm -rfv CCIDDriver_arm64.pkg
pkgbuild --root bundle_arm64 --component-plist CCIDDriver.plist --identifier jp.co.diverta.CCIDDriver --version 1.5.2 --install-location /usr/local/libexec/SmartCardServices/drivers CCIDDriver_arm64.pkg
```

[Intel mac向け]
```
cd ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/
rm -rfv CCIDDriver_x86.pkg
pkgbuild --root bundle_x86 --component-plist CCIDDriver.plist --identifier jp.co.diverta.CCIDDriver --version 1.5.2 --install-location /usr/local/libexec/SmartCardServices/drivers CCIDDriver_x86.pkg
```

下記は実行例になります。<br>
`CCIDDriver_xxx.pkg`というファイル（CCIDドライバーのインストーラー）が作成されます。<br>
この`CCIDDriver_xxx.pkg`を、適宜ほかのmacOS環境に配布し、CCIDドライバーをインストールすることになります。

```
bash-3.2$ cd ${HOME}/GitHub/onecard-fido/CCID/macOSDriver/
bash-3.2$ rm -rfv CCIDDriver_arm64.pkg
bash-3.2$ pkgbuild --root bundle_arm64 --component-plist CCIDDriver.plist --identifier jp.co.diverta.CCIDDriver --version 1.5.2 --install-location /usr/local/libexec/SmartCardServices/drivers CCIDDriver_arm64.pkg
pkgbuild: Reading components from CCIDDriver.plist
pkgbuild: Adding component at ifd-ccid.bundle
pkgbuild: Wrote package to CCIDDriver_arm64.pkg
bash-3.2$
bash-3.2$ ls -al *.pkg
-rw-r--r--  1 devmorit  staff  119701  3  9 17:04 CCIDDriver.pkg
-rw-r--r--  1 devmorit  staff  119699  3  9 17:05 CCIDDriver_arm64.pkg
-rw-r--r--  1 devmorit  staff  120700  3  8 19:03 CCIDDriver_x86.pkg
bash-3.2$
```

以上で、CCIDドライバーのインストーラー作成は完了です。
