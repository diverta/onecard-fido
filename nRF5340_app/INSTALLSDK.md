# nRF Connect SDKインストール手順書

「[nRF Connect SDK v1.8.0](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/)」をmacOSにインストールする手順について掲載します。

## 前提条件

まずは以下の手順書を参照し、現在使用中のnRF5 SDK（nRF5 SDK v17.0.2）による開発環境が整備されていることを前提とします。

#### [NetBeansインストール手順](../nRF52840_app/NETBEANSINST.md)
[nRF52840アプリケーション](../nRF52840_app/README.md)のビルドに必要なツールを準備します。<br>
具体的には、nRF Connect SDKを新規導入するために最低限必要な、以下のツール群をインストールします。
- nRFコマンドラインツール（nrfjprogコマンドが含まれる）
- ARM GCCツールチェイン
- SEGGER J-Link

#### [CMakeインストール手順](../nRF5340_app/INSTALLCMAKE.md)
メイクファイル生成コマンド「cmake」を含むツール「CMake」を準備します。

## Python3環境の準備

<b>macOSに同梱のPython 3.7を使用</b>し、Python3の仮想環境を作成します。<br>
必要なPythonライブラリー（モジュール）は、全て仮想環境下にインストールします。

#### 仮想環境の作成

本例では、`${HOME}/opt/ncs_1.8.0-rc2`というフォルダーに、Python3の仮想環境を作成するものとします。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt
/usr/bin/python3 -m venv ncs_1.8.0-rc2
```

以下は実行例になります。

```
bash-3.2$ /usr/bin/python3 --version
Python 3.7.3
bash-3.2$ cd opt
bash-3.2$ pwd
/Users/makmorit/opt
bash-3.2$
bash-3.2$ /usr/bin/python3 -m venv ncs_1.8.0-rc2
bash-3.2$
```

#### 仮想環境に入る

仮想環境に入るためには、コマンド`cd ${HOME}/opt/ncs_1.8.0-rc2;source bin/activate`を実行します。

```
bash-3.2$ cd ${HOME}/opt/ncs_1.8.0-rc2;source bin/activate
(ncs_1.8.0-rc2) bash-3.2$
```

`(ncs_1.8.0-rc2) bash-3.2$`というコマンドプロンプト表示により、仮想環境に入ったことが確認できます。

#### 仮想環境から抜ける

仮想環境から通常のシェルに戻るためには、コマンド`deactivate`を実行します。

```
(ncs_1.8.0-rc2) bash-3.2$ deactivate
bash-3.2$
```

`bash-3.2$`というコマンドプロンプト表示により、仮想環境を抜け、通常のシェルに戻ったことが確認できます。

## westツールの更新

仮想環境に入り、コマンド`pip3 install west`を実行し、最新のwestツールを取得します。

```
(ncs_1.8.0-rc2) bash-3.2$ pip3 install west
Collecting west
  Downloading https://files.pythonhosted.org/packages/8e/0c/9f8ee26eb8b27b3aae9e2a2b6ec7cd72ba119c234548ceee184c2df700a2/west-0.12.0-py3-none-any.whl (84kB)
    100% |████████████████████████████████| 92kB 1.4MB/s
Requirement already satisfied: setuptools in ./lib/python3.7/site-packages (from west) (40.8.0)
Collecting pykwalify (from west)
  Using cached https://files.pythonhosted.org/packages/1f/fd/ac2161cce19fd67a18c269073f8e86292b5511acec6f8ef6eab88615d032/pykwalify-1.8.0-py2.py3-none-any.whl
:
  Running setup.py install for docopt ... done
Successfully installed PyYAML-6.0 colorama-0.4.4 docopt-0.6.2 packaging-21.3 pykwalify-1.8.0 pyparsing-3.0.7 python-dateutil-2.8.2 ruamel.yaml-0.17.20 ruamel.yaml.clib-0.2.6 six-1.16.0 west-0.12.0
You are using pip version 19.0.3, however version 21.3.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs_1.8.0-rc2) bash-3.2$
```

## nRF Connect SDKのインストール

前述のwestツールを使用し、nRF Connect SDKのインストールを行います。<br>
こちらも、あらかじめ仮想環境に入った上で実施してください。

参考：https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/gs_installing.html#get-the-ncs-code

#### リポジトリーのチェックアウト

GitHubリポジトリーから「nRF Connect SDK v1.8.0-rc2」の全ファイルイメージをチェックアウトします。<br>
ターミナルから以下のコマンドを実行します。（実行例は<b>[こちら](assets01/west.log)</b>）

（注：`west zephyr-export`の実行前に、`cmake`コマンドへの実行パスを通しています）

```
west init -m https://github.com/nrfconnect/sdk-nrf --mr v1.8.0-rc2
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
(ncs_1.8.0-rc2) bash-3.2$ cd zephyr
(ncs_1.8.0-rc2) bash-3.2$ west completion bash > ../west-completion.bash
(ncs_1.8.0-rc2) bash-3.2$ cd ..
(ncs_1.8.0-rc2) bash-3.2$ pwd
/Users/makmorit/opt/ncs_1.8.0-rc2
(ncs_1.8.0-rc2) bash-3.2$ ls -al
total 120
drwxr-xr-x  17 makmorit  staff    544  1 25 10:30 .
drwxr-xr-x  12 makmorit  staff    384  1 25 10:03 ..
:
-rw-r--r--   1 makmorit  staff  17789  1 25 10:30 west-completion.bash
drwxr-xr-x  46 makmorit  staff   1472  1 25 10:05 zephyr
(ncs_1.8.0-rc2) bash-3.2$
```

以上で、nRF Connect SDKのインストールは完了です。
