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

## Python3環境の準備

macOSに同梱のPython 3.7を使用し、Python3の仮想環境を作成します。<br>
必要なPythonライブラリー（モジュール）は、全て仮想環境下にインストールします。

#### 仮想環境の作成

本例では、`${HOME}/GitHub/onecard-fido/pyvenvs/ncs`というフォルダーに、Python3の仮想環境を作成するものとします。<br>
以下のコマンドを実行します。

```
cd ${HOME}/GitHub/onecard-fido/pyvenvs
/usr/bin/python3 -m venv ncs
```

以下は実行例になります。

```
bash-3.2$ cd ${HOME}/GitHub/onecard-fido/pyvenvs
bash-3.2$ /usr/bin/python3 -m venv ncs
bash-3.2$ ls -al
total 8
drwxr-xr-x   4 makmorit  staff  128  3 29 12:17 .
drwxr-xr-x  17 makmorit  staff  544  3 29 12:14 ..
drwxr-xr-x   6 makmorit  staff  192  3 29 12:17 ncs
-rw-r--r--@  1 makmorit  staff  343  3 29 11:57 requirements_ncs.txt
bash-3.2$
```

#### 仮想環境に入る

仮想環境に入るためには、コマンド`cd ${HOME}/GitHub/onecard-fido/pyvenvs/ncs;source bin/activate`を実行します。

```
bash-3.2$ cd ${HOME}/GitHub/onecard-fido/pyvenvs/ncs;source bin/activate
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

nRF Connect SDKの依存ライブラリーを、前述の仮想環境にインストールします。

ターミナルから、`pip3 install -r ${HOME}/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt`を実行してインストールします。<br>
あらかじめ仮想環境に入った上で実施してください。

以下はmacOS Catalinaでの実行例になります。

```
bash-3.2$ cd ${HOME}/GitHub/onecard-fido/pyvenvs/ncs;source bin/activate
(ncs) bash-3.2$ pip3 install -r ${HOME}/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt
Collecting cbor==1.0.0 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 1))
  Using cached https://files.pythonhosted.org/packages/9b/99/01c6a987c920500189eb74a291bd3a388e6c7cf85736bb6b066d9833315e/cbor-1.0.0.tar.gz
Collecting cffi==1.14.5 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 2))
  Using cached https://files.pythonhosted.org/packages/10/f3/d11dfd315ae16f465a815e8746aafe3e4c7905e8cadf26dc04b1fe201fb6/cffi-1.14.5-cp37-cp37m-macosx_10_9_x86_64.whl
Collecting click==7.1.2 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 3))
  Using cached https://files.pythonhosted.org/packages/d2/3d/fa76db83bf75c4f8d338c2fd15c8d33fdd7ad23a9b5e57eb6c5de26b430e/click-7.1.2-py2.py3-none-any.whl
Collecting colorama==0.4.4 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 4))
  Using cached https://files.pythonhosted.org/packages/44/98/5b86278fbbf250d239ae0ecb724f8572af1c91f4a11edf4d36a206189440/colorama-0.4.4-py2.py3-none-any.whl
Collecting cryptography==3.4.6 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 5))
  Using cached https://files.pythonhosted.org/packages/32/52/4ba2bdec39b51a072a968c2b425c3649777d4816d27832703e082e2c5534/cryptography-3.4.6-cp36-abi3-macosx_10_10_x86_64.whl
Collecting docopt==0.6.2 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 6))
  Using cached https://files.pythonhosted.org/packages/a2/55/8f8cab2afd404cf578136ef2cc5dfb50baa1761b68c9da1fb1e4eed343c9/docopt-0.6.2.tar.gz
Collecting ecdsa==0.16.1 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 7))
  Using cached https://files.pythonhosted.org/packages/98/16/70be2716e24eaf5d81074bb3c05429d60292c2a96613a78ac3d69526ad2a/ecdsa-0.16.1-py2.py3-none-any.whl
Collecting imgtool==1.7.2 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 8))
  Using cached https://files.pythonhosted.org/packages/c0/9e/fd919b6df0979f3d05094753bcd954ee0d4239544e11963068eaed2a7a98/imgtool-1.7.2-py3-none-any.whl
Collecting intelhex==2.3.0 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 9))
  Using cached https://files.pythonhosted.org/packages/97/78/79461288da2b13ed0a13deb65c4ad1428acb674b95278fa9abf1cefe62a2/intelhex-2.3.0-py2.py3-none-any.whl
Collecting ninja==1.10.0.post2 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 10))
  Using cached https://files.pythonhosted.org/packages/a1/4f/619a45eb3fa24d11a297790015dcc212ab85287ad39e744e2d5d3b028280/ninja-1.10.0.post2-py3-none-macosx_10_6_x86_64.whl
Collecting packaging==20.9 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 11))
  Using cached https://files.pythonhosted.org/packages/3e/89/7ea760b4daa42653ece2380531c90f64788d979110a2ab51049d92f408af/packaging-20.9-py2.py3-none-any.whl
Collecting pycparser==2.20 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 12))
  Using cached https://files.pythonhosted.org/packages/ae/e7/d9c3a176ca4b02024debf82342dab36efadfc5776f9c8db077e8f6e71821/pycparser-2.20-py2.py3-none-any.whl
Collecting pyelftools==0.27 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 13))
  Using cached https://files.pythonhosted.org/packages/6f/50/3d7729d64bb23393aa4c166af250a6e6f9def40c90bf0e9af3c5ad25b6f7/pyelftools-0.27-py2.py3-none-any.whl
Collecting pykwalify==1.8.0 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 14))
  Using cached https://files.pythonhosted.org/packages/1f/fd/ac2161cce19fd67a18c269073f8e86292b5511acec6f8ef6eab88615d032/pykwalify-1.8.0-py2.py3-none-any.whl
Collecting pyparsing==2.4.7 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 15))
  Using cached https://files.pythonhosted.org/packages/8a/bb/488841f56197b13700afd5658fc279a2025a39e22449b7cf29864669b15d/pyparsing-2.4.7-py2.py3-none-any.whl
Collecting python-dateutil==2.8.1 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 16))
  Using cached https://files.pythonhosted.org/packages/d4/70/d60450c3dd48ef87586924207ae8907090de0b306af2bce5d134d78615cb/python_dateutil-2.8.1-py2.py3-none-any.whl
Collecting PyYAML==5.4.1 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 17))
  Using cached https://files.pythonhosted.org/packages/6b/8c/674cc47282af12bd7f12eea6cc87d907ada593b15f5ba0b51638599500c9/PyYAML-5.4.1-cp37-cp37m-macosx_10_9_x86_64.whl
Collecting ruamel.yaml==0.16.13 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 18))
  Using cached https://files.pythonhosted.org/packages/ed/c3/4c823dac2949a6baf36a4987d04c50d30184147393ba6f4bfb4c67d15a13/ruamel.yaml-0.16.13-py2.py3-none-any.whl
Collecting ruamel.yaml.clib==0.2.2 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 19))
  Using cached https://files.pythonhosted.org/packages/bb/b1/97e99a63735273315ca9c81099c537025678c1709c7a12acf3fe4d7bf5ea/ruamel.yaml.clib-0.2.2-cp37-cp37m-macosx_10_9_x86_64.whl
Collecting six==1.15.0 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 20))
  Using cached https://files.pythonhosted.org/packages/ee/ff/48bde5c0f013094d729fe4b0316ba2a24774b3ff1c52d924a8a4cb04078a/six-1.15.0-py2.py3-none-any.whl
Collecting west==0.10.1 (from -r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 21))
  Using cached https://files.pythonhosted.org/packages/8e/12/7ae664d35278dcdd71317a7f8e4e45d48007276c5fd83ad2da0550594cc0/west-0.10.1-py3-none-any.whl
Requirement already satisfied: setuptools in ./lib/python3.7/site-packages (from west==0.10.1->-r /Users/makmorit/GitHub/onecard-fido/pyvenvs/requirements_ncs.txt (line 21)) (40.8.0)
Installing collected packages: cbor, pycparser, cffi, click, colorama, cryptography, docopt, six, ecdsa, intelhex, imgtool, ninja, pyparsing, packaging, pyelftools, python-dateutil, ruamel.yaml.clib, ruamel.yaml, pykwalify, PyYAML, west
  Running setup.py install for cbor ... done
  Running setup.py install for docopt ... done
Successfully installed PyYAML-5.4.1 cbor-1.0.0 cffi-1.14.5 click-7.1.2 colorama-0.4.4 cryptography-3.4.6 docopt-0.6.2 ecdsa-0.16.1 imgtool-1.7.2 intelhex-2.3.0 ninja-1.10.0.post2 packaging-20.9 pycparser-2.20 pyelftools-0.27 pykwalify-1.8.0 pyparsing-2.4.7 python-dateutil-2.8.1 ruamel.yaml-0.16.13 ruamel.yaml.clib-0.2.2 six-1.15.0 west-0.10.1
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
(ncs) bash-3.2$ pwd
/Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs
(ncs) bash-3.2$ west init -m https://github.com/nrfconnect/sdk-nrf
=== Initializing in /Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs
--- no --manifest-rev was given; using remote's default branch: refs/heads/master
--- Cloning manifest repository from https://github.com/nrfconnect/sdk-nrf, rev. refs/heads/master
Initialized empty Git repository in /Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs/.west/manifest-tmp/.git/
remote: Enumerating objects: 46, done.
：
HEAD is now at 6040e6e1 manifest: update homekit
--- setting manifest.path to nrf
=== Initialized. Now run "west update" inside /Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs.
(ncs) bash-3.2$
(ncs) bash-3.2$ west update
=== updating zephyr (zephyr):
--- zephyr: initializing
Initialized empty Git repository in /Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs/zephyr/.git/
--- zephyr: fetching, need revision 54dea0b2b5309098c388a836fe9b152318592495
remote: Enumerating objects: 542501, done.
remote: Total 542501 (delta 0), reused 0 (delta 0), pack-reused 542501
Receiving objects: 100% (542501/542501), 354.94 MiB | 1.38 MiB/s, done.
：
From https://github.com/zephyrproject-rtos/edtt
 * [new branch]      public_master -> refs/west/public_master
HEAD is now at 7dd56fc Add FIXME note to handle incorrect LL/CON/MAS/BV-27-C implementation
HEAD is now at 7dd56fc Add FIXME note to handle incorrect LL/CON/MAS/BV-27-C implementation
(ncs) bash-3.2$
(ncs) bash-3.2$ PATH=${PATH}:/Applications/CMake.app/Contents/bin
(ncs) bash-3.2$ west zephyr-export
Zephyr (/Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs/zephyr/share/zephyr-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/Zephyr

ZephyrUnittest (/Users/makmorit/GitHub/onecard-fido/pyvenvs/ncs/zephyr/share/zephyrunittest-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/ZephyrUnittest

(ncs) bash-3.2$
```

#### westコマンドのカスタマイズ

`west`コマンドに対し、ビルド用サブコマンド追加等のカスタマイズを行います。<br>
Zephyrフォルダー配下で、コマンド`west completion bash`を実行します。

以下は実行例になります。

```
(ncs) bash-3.2$ cd ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/zephyr
(ncs) bash-3.2$ west completion bash > ${HOME}/GitHub/onecard-fido/pyvenvs/ncs/west-completion.bash
(ncs) bash-3.2$ cd ${HOME}/GitHub/onecard-fido/pyvenvs/ncs
(ncs) bash-3.2$ ls -al
total 64
drwxr-xr-x  17 makmorit  staff    544  3 29 12:42 .
drwxr-xr-x   5 makmorit  staff    160  3 29 12:26 ..
-rw-r--r--@  1 makmorit  staff   6148  3 29 12:38 .DS_Store
drwxr-xr-x   3 makmorit  staff     96  3 29 12:27 .west
drwxr-xr-x  24 makmorit  staff    768  3 29 12:25 bin
drwxr-xr-x   4 makmorit  staff    128  3 29 12:36 bootloader
drwxr-xr-x   2 makmorit  staff     64  3 29 12:17 include
drwxr-xr-x   3 makmorit  staff     96  3 29 12:17 lib
drwxr-xr-x  27 makmorit  staff    864  3 29 12:36 mbedtls
drwxr-xr-x  10 makmorit  staff    320  3 29 12:39 modules
drwxr-xr-x  34 makmorit  staff   1088  3 29 12:27 nrf
drwxr-xr-x  25 makmorit  staff    800  3 29 12:35 nrfxlib
-rw-r--r--   1 makmorit  staff    111  3 29 12:21 pyvenv.cfg
drwxr-xr-x   3 makmorit  staff     96  3 29 12:36 test
drwxr-xr-x   4 makmorit  staff    128  3 29 12:39 tools
-rw-r--r--   1 makmorit  staff  17789  3 29 12:42 west-completion.bash
drwxr-xr-x  48 makmorit  staff   1536  3 29 12:34 zephyr
(ncs) bash-3.2$
```


以上で、nRF Connect SDKのインストールは完了です。
