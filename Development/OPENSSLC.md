# OpenSSL C言語ライブラリー導入手順

## 概要

U2F管理ツール内で秘密鍵ファイル生成などを行うために必要な、OpenSSL C言語ライブラリーの導入手順を、下記に掲載いたします。

macOSとWindowsでは、導入手順が異なるため、個別に手順を掲載することとします。

## macOSへの導入

macOSにOpenSSLを導入し、Xcodeプロジェクトに組み込むまでの手順は以下になります。

### インストール

#### インストール媒体の取得

ブラウザーで[OpenSSLのサイト](https://www.openssl.org/source/)を開きます。

<img src="assets/0015.png" width="600">

インストール媒体（openssl-1.1.0g.tar.gzというファイル）を、リンクをクリックしてダウンロードします。

ダウンロードしたファイルを解凍すると、下図のようなファイルが格納されています。

<img src="assets/0016.png" width="500">

同梱の INSTALL というファイルを開くと、導入方法が記載されています。
```
 Quick Start
 -----------

 If you want to just get on with it, do:

  on Unix:

    $ ./config
    $ make
    $ make test
    $ make install
```

#### インストールの実行

前述INSTALLの内容を参考に、インストールのためのコマンドを下記の通り実行します。<br>
（今回は、インストール先をユーザーディレクトリー配下に指定しております）

```
$ ./config --prefix=$HOME/openssl
$ make
$ make test
$ make install
```
ご参考：[上記コマンド実行時のログ](assets/openssl.make.log)

### Xcodeプロジェクトへの組込み

ユーザーディレクトリー配下にできたインクルードファイルと、libcrypto.aを、開発時に使用します。
macOS版の場合、U2FMaintenanceTool.appのXcodeプロジェクトに組み込んで使用することになります。

```
MacBookPro-makmorit-jp:lib makmorit$ pwd
/Users/makmorit/openssl/lib
MacBookPro-makmorit-jp:lib makmorit$ ls -al *.a
-rw-r--r--  1 makmorit  staff  3718816 Feb 15 13:16 libcrypto.a <---これを使用
-rw-r--r--  1 makmorit  staff   571720 Feb 15 13:16 libssl.a
MacBookPro-makmorit-jp:lib makmorit$
```

#### ヘッダーファイルの追加指定

XcodeのHeader Search Pathsに、OpenSSLのインクルードディレクトリーを下記のように指定します。

<img src="assets/0017.png" width="900">

#### ライブラリーファイルの追加指定

先述のlibcrypto.aのアイコンを、Xcode左側にあるツリービューの、Frameworks/Other Frameworksというフォルダーにドラッグ＆ドロップします。

<img src="assets/0018.png" width="200">

下図のようなダイアログがポップアップ表示されますので、そのままFinishをクリックします。

<img src="assets/0019.png" width="500">

libcrypto.aがツリービューに追加されます。

<img src="assets/0020.png" width="200">

XcodeのLibrary Search Pathsに、（libcrypto.aが配置されている）OpenSSLのライブラリーディレクトリーを下記のように指定します。

<img src="assets/0021.png" width="900">

#### 指定したパスの確認

下図のように、指定したパスが自動計算されて表示されることを確認します。

<img src="assets/0022.png" width="900">

#### サンプルコードによる確認

下図のようなサンプルコードを入れた後、プロダクトのビルドを行います。<br>
エラーが発生しなければ、導入は成功です。

<img src="assets/0023.png" width="400">

以上でXcodeプロジェクトへの組込みは完了です。
