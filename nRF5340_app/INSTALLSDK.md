# nRF Connect SDKインストール手順書

最終更新日：2023/02/14

「[nRF Connect SDK v2.2.0](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.2.0/nrf/)」をmacOSにインストールする手順について掲載します。

## 使用したシステム

PC: iMac (Retina 5K, 27-inch, 2019)<br>
OS: macOS 12.6.3

## 前提条件

まずは下記手順書により、各種ソフトウェアがインストールされていることを前提とします。<br>

- <b>[ARM GCCインストール手順](../nRF52840_app/ARMGCCINST.md)</b><br>
コンパイル、リンク等を実行するためのコマンドラインツール群がインストールされます。

- <b>[Homebrew＋ツール群インストール手順](../nRF5340_app/INSTALLBREWTOOL.md)</b><br>
パッケージ管理ツール「Homebrew」およびnRF Connect SDKに必要なツール群がインストールされます。

## Python環境の準備

Homebrewでインストールした<b>の最新版のPython 3.8</b>を使用し、Python仮想環境を作成します。<br>
必要なPythonライブラリー（モジュール）は、全て仮想環境下にインストールします。

#### 仮想環境の作成

本例では、`${HOME}/opt/ncs_2.2.0`というフォルダーに、Pythonの仮想環境を作成するものとします。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt
python -m venv ncs_2.2.0
```

以下は実行例になります。

```
bash-3.2$ python --version
Python 3.8.16
bash-3.2$ cd ${HOME}/opt
bash-3.2$ python -m venv ncs_2.2.0
bash-3.2$
```

#### 仮想環境に入る

仮想環境に入るためには、仮想環境フォルダーでコマンド`source bin/activate`を実行します。

```
bash-3.2$ cd ${HOME}/opt/ncs_2.2.0
bash-3.2$ source bin/activate
(ncs_2.2.0) bash-3.2$
```

`(ncs_2.2.0) bash-3.2$ `というコマンドプロンプト表示により、仮想環境に入ったことが確認できます。

#### 仮想環境から抜ける

仮想環境から通常のシェルに戻るためには、コマンド`deactivate`を実行します。

```
(ncs_2.2.0) bash-3.2$ deactivate
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

参考：https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.2.0/nrf/gs_installing.html#get-the-ncs-code

#### リポジトリーのチェックアウト

GitHubリポジトリーから「nRF Connect SDK v2.2.0」の全ファイルイメージをチェックアウトします。<br>
ターミナルから以下のコマンドを実行します。（実行例は<b>[こちら](assets01/west.log)</b>）

```
west init -m https://github.com/nrfconnect/sdk-nrf --mr v2.2.0
west update
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

以上で、nRF Connect SDKのインストールは完了です。
