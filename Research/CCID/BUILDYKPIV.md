
# 参考：Yubico PIV Toolビルド手順

PIVカード管理ツールとして、ソースコードが公開されているYubico PIV Toolを、macOS環境上で利用できるよう、ビルド手順を整理しました。

Yubico PIV Tool (command line) のソースコードを参照することにより、PCのプログラムから、どのようにしてPIVデバイスに接続／データ転送を行なっているのかを参照することができると思われます。

## ビルド手順

#### 前提パッケージを導入

前提となるパッケージを、brewなどで事前にインストールしておきます。<br>
当方の環境では`check`なるツールが不足していたので、追加インストールしました。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ brew install check
Updating Homebrew...
==> Auto-updated Homebrew!
Updated 1 tap (homebrew/core).
（中略）
==> Downloading https://github.com/libcheck/check/releases/download/0.14.0/check-0.14.0.tar.gz
==> Downloading from https://github-production-release-asset-2e65be.s3.amazonaws.com/48520045/5eb1d580-4022-11ea-9ec5-7d
######################################################################## 100.0%
==> ./configure --prefix=/usr/local/Cellar/check/0.14.0
==> make install
🍺  /usr/local/Cellar/check/0.14.0: 42 files, 528.5KB, built in 31 seconds
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ echo $?
0
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

#### ソースコードを取得

下記サイトから yubico-piv-tool-2.0.0.tar.gz をダウンロード後、解凍します。
- <b>[yubico - Smart card drivers and tools](https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/)</b><br>
Yubico PIV Tool (command line) というツールのリンク「Mac OS X download」をクリックしてダウンロードします。

解凍したフォルダー`yubico-piv-tool-2.0.0`を、`${HOME}/opt/`配下に移動します。

```
MacBookPro-makmorit-jp:~ makmorit$ ls -al opt
total 32
drwxr-xr-x   9 makmorit  staff    306  5 15 14:50 .
drwxr-xr-x+ 61 makmorit  staff   2074  5 15 14:32 ..
-rw-r--r--@  1 makmorit  staff  12292  5 15 14:50 .DS_Store
drwxr-xr-x   7 makmorit  staff    238 10  1  2018 gcc-arm-none-eabi-7-2018-q2-update
drwxr-xr-x   5 makmorit  staff    170  6 11  2019 nRF-Command-Line-Tools_9_8_1_OSX
drwxr-xr-x  14 makmorit  staff    476 11  7  2018 nRF5_SDK_15.2.0
drwxr-xr-x  14 makmorit  staff    476  4 29 10:35 nRF5_SDK_15.3.0
drwxr-xr-x   5 makmorit  staff    170 10  1  2018 nRF5x-Command-Line-Tools_9_7_3_OSX
drwxr-xr-x@ 25 makmorit  staff    850  5 15 14:55 yubico-piv-tool-2.0.0
MacBookPro-makmorit-jp:~ makmorit$
```

#### メイクファイルの生成

ソースコードフォルダー`yubico-piv-tool-2.0.0`に移動したら、シェル`configure`を実行し、メイクファイルを生成します。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig" ./configure
checking for a BSD-compatible install... /usr/bin/install -c
checking whether build environment is sane... yes
checking for a thread-safe mkdir -p... build-aux/install-sh -c -d
（中略）
configure: summary of build options:

  Version:          2.0.0
  Host type:        x86_64-apple-darwin16.7.0
  Install prefix:   /usr/local
  Compiler:         gcc
  CFLAGS:           -g -O2
  CPPFLAGS:         
  Warnings:         
  Backend:          macscard
  OpenSSL version:  1.1.1
  PCSC
          CFLAGS:   
            LIBS:   
  Winscard
            LIBS:   
  Mac PCSC
            LIBS:   -Wl,-framework -Wl,PCSC
  Custom PCSC
            LIBS:   

  YKCS11 debug:    DISABLED
  Hardware tests:  DISABLED

MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

#### ビルドの実行

`make`コマンドを実行し、ビルドを実行します。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ make
Making all in lib
Making all in .
  CC       ykpiv.lo
  CC       util.lo
  CC       internal.lo
  CC       version.lo
  CC       error.lo
  CCLD     libykpiv.la
Making all in tests
make[2]: Nothing to be done for `all'.
Making all in tool
/Applications/Xcode.app/Contents/Developer/usr/bin/make  all-recursive
Making all in .
（中略）
Making all in tests
make[2]: Nothing to be done for `all'.
make[1]: Nothing to be done for `all-am'.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ echo $?
0
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

以上でビルドは完了になります。

[注] 本件調査では、ソースコードと実際の動作を参照するのが目的であるため、システムへのインストールは行わないものといたします。
