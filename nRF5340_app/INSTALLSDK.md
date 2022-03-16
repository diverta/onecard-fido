# nRF Connect SDKインストール手順書

「[nRF Connect SDK v1.9.1](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.9.1/nrf/)」をmacOSにインストールする手順について掲載します。

## 前提条件

まずは下記手順書により、各種ソフトウェアがインストールされていることを前提とします。<br>

- <b>[ARM GCCインストール手順](../nRF52840_app/ARMGCCINST.md)</b><br>
コンパイル、リンク等を実行するためのコマンドラインツール群がインストールされます。

- <b>[CMakeインストール手順](../nRF5340_app/INSTALLCMAKE.md)</b><br>
メイクファイル生成コマンド「`cmake`」を含むツール「CMake」がインストールされます。

## Python3環境の準備

<b>macOSに同梱のPython 3.7を使用</b>し、Python3の仮想環境を作成します。<br>
必要なPythonライブラリー（モジュール）は、全て仮想環境下にインストールします。

#### 仮想環境の作成

本例では、`${HOME}/opt/ncs_1.9.1`というフォルダーに、Python3の仮想環境を作成するものとします。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt
/usr/bin/python3 -m venv ncs_1.9.1
```

以下は実行例になります。

```
bash-3.2$ /usr/bin/python3 --version
Python 3.7.3
bash-3.2$ cd ${HOME}/opt
bash-3.2$
bash-3.2$ /usr/bin/python3 -m venv ncs_1.9.1
bash-3.2$
```

#### 仮想環境に入る

仮想環境に入るためには、仮想環境フォルダーでコマンド`source bin/activate`を実行します。

```
bash-3.2$ cd ncs_1.9.1
bash-3.2$ source bin/activate
(ncs_1.9.1) bash-3.2$
```

`(ncs_1.9.1) bash-3.2$ `というコマンドプロンプト表示により、仮想環境に入ったことが確認できます。

#### 仮想環境から抜ける

仮想環境から通常のシェルに戻るためには、コマンド`deactivate`を実行します。

```
(ncs_1.9.1) bash-3.2$ deactivate
bash-3.2$
```

`bash-3.2$`というコマンドプロンプト表示により、仮想環境を抜け、通常のシェルに戻ったことが確認できます。

## westツールの更新

仮想環境に入り、最新のwestツールを取得します。<br>
以下のコマンドを実行します。（実行例は<b>[こちら](assets01/install_west.log)</b>）

```
pip3 install west
```

## nRF Connect SDKのインストール

前述のwestツールを使用し、nRF Connect SDKのインストールを行います。<br>
こちらも、あらかじめ仮想環境に入った上で実施してください。

参考：https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.9.1/nrf/gs_installing.html#get-the-ncs-code

#### リポジトリーのチェックアウト

GitHubリポジトリーから「nRF Connect SDK v1.9.1」の全ファイルイメージをチェックアウトします。<br>
ターミナルから以下のコマンドを実行します。（実行例は<b>[こちら](assets01/west.log)</b>）

（注：`west zephyr-export`の実行前に、`cmake`コマンドへの実行パスを通しています）

```
west init -m https://github.com/nrfconnect/sdk-nrf --mr v1.9.1
west update
PATH=${PATH}:/Applications/CMake.app/Contents/bin
west zephyr-export
```

#### 依存ライブラリーの導入

nRF Connect SDKの依存ライブラリーを、前述の仮想環境にインストールします。<br>
以下のコマンドを実行します。（実行例は<b>[こちら](assets01/pip3.log)</b>）

```
pip3 install -r zephyr/scripts/requirements.txt
pip3 install -r nrf/scripts/requirements.txt
pip3 install -r bootloader/mcuboot/scripts/requirements.txt
```

#### westコマンドのカスタマイズ

`west`コマンドに対し、ビルド用サブコマンド追加等のカスタマイズを行います。<br>
`Zephyr`フォルダー配下で、コマンド`west completion bash`を実行します。

以下は実行例になります。

```
(ncs_1.9.1) bash-3.2$ cd zephyr
(ncs_1.9.1) bash-3.2$ west completion bash > ../west-completion.bash
(ncs_1.9.1) bash-3.2$ cd ..
(ncs_1.9.1) bash-3.2$ ls -al
total 120
drwxr-xr-x  17 makmorit  staff    544  3 16 09:58 .
drwxr-xr-x  15 makmorit  staff    480  3 16 09:47 ..
drwxr-xr-x   3 makmorit  staff     96  3 16 09:49 .west
：
-rw-r--r--   1 makmorit  staff  17789  3 16 09:58 west-completion.bash
drwxr-xr-x  47 makmorit  staff   1504  3 16 09:50 zephyr
(ncs_1.9.1) bash-3.2$
(ncs_1.9.1) bash-3.2$ deactivate
bash-3.2$
```

以上で、nRF Connect SDKのインストールは完了です。
