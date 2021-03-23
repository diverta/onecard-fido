# nRF Connect SDKインストール手順書

「[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)」をmacOSにインストールする手順について掲載します。

## 前提条件

まずは以下の手順書を参照し、現在使用中のSDK（nRF5 SDK v15.3.0）による開発環境が整備されていることを前提とします。

#### [NetBeansインストール手順](../../nRF52840_app/NETBEANSINST.md)
[nRF52840アプリケーション](../../nRF52840_app/README.md)のビルドに必要なツールを準備します。<br>
具体的には、nRF Connect SDKを新規導入するために最低限必要な、以下のツール群をインストールします。
- nRFコマンドラインツール（nrfjprogコマンドが含まれる）
- ARM GCCツールチェイン
- SEGGER J-Link

#### [nRF Connect for Desktop導入手順](../../nRF52840_app/NRFCONNECTINST.md)
nRF Connect SDKインストール時に必要なGUIツールを準備します。

#### [CMakeインストール手順](../../Research/nRFCnctSDK_v1.4.99/INSTALLCMAKE.md)
メイクファイル生成コマンド「cmake」を含むツール「CMake」を準備します。

## 前提ソフトウェアのインストール

macOSに、事前に`ninja`というツールをインストールしておきます。

#### ninjaのインストール

macOSのターミナルから以下のコマンドを実行します。

```
brew install ninja
```

以下はmacOS Catalinaでの実行例になります。

```
bash-3.2$ brew install ninja
Updating Homebrew...
：
==> Installing dependencies for ninja: gdbm, openssl@1.1, readline, sqlite, xz and python@3.9
==> Installing ninja dependency: gdbm
：
==> Summary
🍺  /usr/local/Cellar/python@3.9/3.9.1_3: 3,895 files, 63.9MB
==> Installing ninja
==> Pouring ninja-1.10.2.catalina.bottle.tar.gz
==> Caveats
zsh completions have been installed to:
  /usr/local/share/zsh/site-functions
==> Summary
🍺  /usr/local/Cellar/ninja/1.10.2: 7 files, 352.1KB
==> `brew cleanup` has not been run in 30 days, running now...
：
==> python@3.9
Python has been installed as
  /usr/local/bin/python3

Unversioned symlinks `python`, `python-config`, `pip` etc. pointing to
`python3`, `python3-config`, `pip3` etc., respectively, have been installed into
  /usr/local/opt/python@3.9/libexec/bin

You can install Python packages with
  pip3 install <package>
They will install into the site-package directory
  /usr/local/lib/python3.9/site-packages

See: https://docs.brew.sh/Homebrew-and-Python
==> ninja
zsh completions have been installed to:
  /usr/local/share/zsh/site-functions
bash-3.2$
```

途中、Python 3.9が強制的にインストールされるケースがあります。<br>
この場合は上記のように、その旨のログが出力されますので、ご注意願います。

#### Getting Started Assistantのインストール（ご参考）

本手順に必須ではありませんが、開発環境（IDE）として「[SEGGER Embedded Studio](https://www.nordicsemi.com/Software-and-tools/Development-Tools/Segger-Embedded-Studio)」の使用を検討している場合、このツールがあると非常に便利であるため、適宜インストールしておきます。

nRF Connect for Desktopを起動すると、下図のように「Getting Started Assistant」がリストアップされます。<br>
右側の「Install」ボタンをクリックします。

<img src="assets01/0006.jpg" width="300">

インストール処理が完了すると、画面のリストが下図のような状態に変わります。<br>
右側の「Open」ボタンをクリックします。

<img src="assets01/0007.jpg" width="300">

下図のような「Getting Started Assistant」画面が起動することを確認します。

<img src="assets01/0008.jpg" width="420">

以上で、Getting Started Assistantのインストールは完了です。
## Python3環境の準備

macOSに同梱のPython 3.7を使用し、Python3の仮想環境を作成します。<br>
必要なPythonライブラリー（モジュール）は、全て仮想環境下にインストールします。

#### 仮想環境の作成

本例では、`${HOME}/opt/venv/ncs`というフォルダーに、Python3の仮想環境を作成するものとします。<br>
以下のコマンドを実行します。

```
mkdir -p ${HOME}/opt/venv
cd ${HOME}/opt/venv
/usr/bin/python3 -m venv ncs
```

以下は実行例になります。

```
bash-3.2$ mkdir -p ${HOME}/opt/venv
bash-3.2$ cd ${HOME}/opt/venv
bash-3.2$ pwd
/Users/makmorit/opt/venv
bash-3.2$
bash-3.2$ /usr/bin/python3 -m venv ncs
bash-3.2$ ls -al
total 0
drwxr-xr-x   3 makmorit  staff   96  3 22 11:00 .
drwxr-xr-x  11 makmorit  staff  352  3 22 10:48 ..
drwxr-xr-x   6 makmorit  staff  192  3 22 11:00 ncs
bash-3.2$
```

#### 仮想環境に入る

仮想環境に入るためには、コマンド`cd ${HOME}/opt/venv/ncs;source bin/activate`を実行します。

```
bash-3.2$ cd ${HOME}/opt/venv/ncs;source bin/activate
(ncs) bash-3.2$
```

`(ncs) bash-3.2$`というコマンドプロンプト表示により、仮想環境に入ったことが確認できます。

#### 仮想環境から抜ける

仮想環境から通常のシェルに戻るためには、コマンド`deactivate`を実行します。

```
(ncs) bash-3.2$ deactivate
bash-3.2$
```

`bash-3.2$`というコマンドプロンプト表示により、仮想環境を抜け、通常のシェルに戻ったことが確認できます。

## 依存ライブラリーの導入

以下の依存ライブラリーを、前述の仮想環境にインストールします。
- west
- pyelftools
- click
- cryptography
- cbor
- intelhex
- ecdsa

ターミナルから、`pip3 install <ライブラリー名>`を実行してインストールします。<br>
あらかじめ仮想環境に入った上で実施してください。

以下はmacOS Catalinaでの実行例になります。

```
(ncs) bash-3.2$ pip3 install west
Collecting west
  Downloading https://files.pythonhosted.org/packages/8e/12/7ae664d35278dcdd71317a7f8e4e45d48007276c5fd83ad2da0550594cc0/west-0.10.1-py3-none-any.whl (81kB)
    100% |████████████████████████████████| 81kB 851kB/s
：
Collecting ruamel.yaml.clib>=0.1.2; platform_python_implementation == "CPython" and python_version < "3.10" (from ruamel.yaml>=0.16.0->pykwalify->west)
  Downloading https://files.pythonhosted.org/packages/bb/b1/97e99a63735273315ca9c81099c537025678c1709c7a12acf3fe4d7bf5ea/ruamel.yaml.clib-0.2.2-cp37-cp37m-macosx_10_9_x86_64.whl (147kB)
    100% |████████████████████████████████| 153kB 1.3MB/s
Installing collected packages: PyYAML, colorama, pyparsing, packaging, six, python-dateutil, docopt, ruamel.yaml.clib, ruamel.yaml, pykwalify, west
  Running setup.py install for docopt ... done
Successfully installed PyYAML-5.4.1 colorama-0.4.4 docopt-0.6.2 packaging-20.9 pykwalify-1.8.0 pyparsing-2.4.7 python-dateutil-2.8.1 ruamel.yaml-0.16.13 ruamel.yaml.clib-0.2.2 six-1.15.0 west-0.10.1
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$ which west
/Users/makmorit/opt/venv/ncs/bin/west
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install pyelftools
Collecting pyelftools
  Downloading https://files.pythonhosted.org/packages/6f/50/3d7729d64bb23393aa4c166af250a6e6f9def40c90bf0e9af3c5ad25b6f7/pyelftools-0.27-py2.py3-none-any.whl (151kB)
    100% |████████████████████████████████| 153kB 1.1MB/s
Installing collected packages: pyelftools
Successfully installed pyelftools-0.27
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install click
Collecting click
  Using cached https://files.pythonhosted.org/packages/d2/3d/fa76db83bf75c4f8d338c2fd15c8d33fdd7ad23a9b5e57eb6c5de26b430e/click-7.1.2-py2.py3-none-any.whl
Installing collected packages: click
Successfully installed click-7.1.2
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install cryptography
Collecting cryptography
  Downloading https://files.pythonhosted.org/packages/32/52/4ba2bdec39b51a072a968c2b425c3649777d4816d27832703e082e2c5534/cryptography-3.4.6-cp36-abi3-macosx_10_10_x86_64.whl (2.0MB)
    100% |████████████████████████████████| 2.0MB 1.9MB/s
：
Installing collected packages: pycparser, cffi, cryptography
Successfully installed cffi-1.14.5 cryptography-3.4.6 pycparser-2.20
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install cbor
Collecting cbor
  Using cached https://files.pythonhosted.org/packages/9b/99/01c6a987c920500189eb74a291bd3a388e6c7cf85736bb6b066d9833315e/cbor-1.0.0.tar.gz
Installing collected packages: cbor
  Running setup.py install for cbor ... done
Successfully installed cbor-1.0.0
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install intelhex
Collecting intelhex
  Downloading https://files.pythonhosted.org/packages/97/78/79461288da2b13ed0a13deb65c4ad1428acb674b95278fa9abf1cefe62a2/intelhex-2.3.0-py2.py3-none-any.whl (50kB)
    100% |████████████████████████████████| 51kB 645kB/s
Installing collected packages: intelhex
Successfully installed intelhex-2.3.0
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
(ncs) bash-3.2$ pip3 install ecdsa
Collecting ecdsa
  Using cached https://files.pythonhosted.org/packages/98/16/70be2716e24eaf5d81074bb3c05429d60292c2a96613a78ac3d69526ad2a/ecdsa-0.16.1-py2.py3-none-any.whl
Requirement already satisfied: six>=1.9.0 in /Users/makmorit/opt/venv/ncs/lib/python3.7/site-packages (from ecdsa) (1.15.0)
Installing collected packages: ecdsa
Successfully installed ecdsa-0.16.1
You are using pip version 19.0.3, however version 21.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
```

## nRF Connect SDKのインストール

前述のツール「west」を使用し、nRF Connect SDKのインストールを行います。<br>
あらかじめ仮想環境に入った上で実施してください。

#### リポジトリーのチェックアウト

GitHubリポジトリーから、nRF Connect SDKの全ファイルイメージをチェックアウトします。<br>
ターミナルから以下のコマンドを実行します。

（注：`west zephyr-export`の実行前に、`cmake`コマンドへの実行パスを通しています）

```
west init -m https://github.com/nrfconnect/sdk-nrf
west update
PATH=${PATH}:/Applications/CMake.app/Contents/bin
west zephyr-export
```

以下は実行例になります（途中ログを省略しています）。

```
bash-3.2$ cd ${HOME}/opt/venv/ncs;source bin/activate
(ncs) bash-3.2$
(ncs) bash-3.2$ pwd
/Users/makmorit/opt/venv/ncs
(ncs) bash-3.2$
(ncs) bash-3.2$ west init -m https://github.com/nrfconnect/sdk-nrf
=== Initializing in /Users/makmorit/opt/venv/ncs
--- no --manifest-rev was given; using remote's default branch: refs/heads/master
--- Cloning manifest repository from https://github.com/nrfconnect/sdk-nrf, rev. refs/heads/master
Initialized empty Git repository in /Users/makmorit/opt/venv/ncs/.west/manifest-tmp/.git/
remote: Enumerating objects: 17, done.
remote: Counting objects: 100% (17/17), done.
remote: Compressing objects: 100% (15/15), done.
remote: Total 60925 (delta 6), reused 3 (delta 2), pack-reused 60908
Receiving objects: 100% (60925/60925), 29.21 MiB | 1.28 MiB/s, done.
：
HEAD is now at f2faacbd Bluetooth: Mesh: Fix formatting of opcode handlers
--- setting manifest.path to nrf
=== Initialized. Now run "west update" inside /Users/makmorit/opt/venv/ncs.
(ncs) bash-3.2$
(ncs) bash-3.2$ west update
=== updating zephyr (zephyr):
--- zephyr: initializing
Initialized empty Git repository in /Users/makmorit/opt/venv/ncs/zephyr/.git/
--- zephyr: fetching, need revision a2d2a5e169f7aec8d9de16787af99e59c56c09b7
remote: Enumerating objects: 528521, done.
remote: Total 528521 (delta 0), reused 0 (delta 0), pack-reused 528521
Receiving objects: 100% (528521/528521), 350.77 MiB | 1.44 MiB/s, done.
：
HEAD is now at b209a60 Fix use of invalid disconnect reason in test procedures
HEAD is now at b209a60 Fix use of invalid disconnect reason in test procedures
(ncs) bash-3.2$ PATH=${PATH}:/Applications/CMake.app/Contents/bin
(ncs) bash-3.2$ west zephyr-export
Zephyr (/Users/makmorit/opt/venv/ncs/zephyr/share/zephyr-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/Zephyr

ZephyrUnittest (/Users/makmorit/opt/venv/ncs/zephyr/share/zephyrunittest-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/ZephyrUnittest

(ncs) bash-3.2$
```

#### westコマンドのカスタマイズ

`west`コマンドに対し、ビルド用サブコマンド追加等のカスタマイズを行います。<br>
Zephyrフォルダー配下で、コマンド`west completion bash`を実行します。

以下は実行例になります。

```
(ncs) bash-3.2$ cd ${HOME}/opt/venv/ncs/zephyr
(ncs) bash-3.2$ west completion bash > ${HOME}/opt/venv/ncs/west-completion.bash
(ncs) bash-3.2$
```

#### ツールチェイン名の整合

`west update`実行の際、ツールチェイン名に`gnuarmemb`以外を指定すると、`west build`時に失敗してしまう不具合があるようです。<br>
そこで、`${HOME}/opt/venv/ncs/zephyr/cmake/toolchain/`配下の`gnuarmemb`フォルダーを、`gcc-arm-none-eabi-9-2020-q2-update`として複製しておきます。[注1]

```
(ncs) bash-3.2$ cp -pr ${HOME}/opt/venv/ncs/zephyr/cmake/toolchain/gnuarmemb ${HOME}/opt/venv/ncs/zephyr/cmake/toolchain/gcc-arm-none-eabi-9-2020-q2-update
(ncs) bash-3.2$ ls -al ${HOME}/opt/venv/ncs/zephyr/cmake/toolchain/
total 16
drwxr-xr-x  13 makmorit  staff   416  3 23 10:24 .
drwxr-xr-x  37 makmorit  staff  1184  3 23 10:17 ..
-rw-r--r--@  1 makmorit  staff  6148  3 23 10:24 .DS_Store
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 arcmwdt
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 cross-compile
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 espressif
drwxr-xr-x   5 makmorit  staff   160  3 23 09:40 gcc-arm-none-eabi-9-2020-q2-update
drwxr-xr-x   5 makmorit  staff   160  3 23 09:40 gnuarmemb
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 host
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 llvm
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 xcc
drwxr-xr-x   4 makmorit  staff   128  3 23 09:40 xtools
drwxr-xr-x   6 makmorit  staff   192  3 23 09:40 zephyr
(ncs) bash-3.2$
```

[注1] ツールチェイン導入時、フォルダー名が`gcc-arm-none-eabi-9-2020-q2-update`としていたための措置になります。その他の名称である場合、当該名称に合わせるようにします。

以上で、nRF Connect SDKのインストールは完了です。
