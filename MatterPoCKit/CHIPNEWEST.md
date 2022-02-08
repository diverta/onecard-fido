# Matterライブラリー更新手順

最終更新日：2022/02/28

現在開発中のMatterライブラリーを最新版に更新し、サンプルアプリを使用して動作確認する手順について掲載しています。

## 概要

概ね以下の手順でライブラリーの更新／動作確認を実施します。

## nRF Connect SDKの更新

nRF Connect SDKの最新版をチェックアウトします。<br>
同時に、Matterが推奨するバージョン（タグ）と整合させます。

#### Python3仮想環境を再作成
nRF Connect SDKチェックアウト時は、Python3の仮想環境ごと、作り直すのが良いと思われます。

```
bash-3.2$ cd ${HOME}/opt
bash-3.2$ /usr/bin/python3 -m venv ncs_1.8.99
bash-3.2$ cd ${HOME}/opt/ncs_1.8.99;source bin/activate
(ncs_1.8.99) bash-3.2$
```

#### nRF Connect SDKの再導入
nRF Connect SDKの最新版をチェックアウトします。

```
(ncs_1.8.99) bash-3.2$ pip3 install west
Collecting west
  Using cached https://files.pythonhosted.org/packages/8e/0c/9f8ee26eb8b27b3aae9e2a2b6ec7cd72ba119c234548ceee184c2df700a2/west-0.12.0-py3-none-any.whl
Collecting pykwalify (from west)
:
Successfully installed PyYAML-6.0 colorama-0.4.4 docopt-0.6.2 packaging-21.3 pykwalify-1.8.0 pyparsing-3.0.7 python-dateutil-2.8.2 ruamel.yaml-0.17.20 ruamel.yaml.clib-0.2.6 six-1.16.0 west-0.12.0
You are using pip version 19.0.3, however version 22.0.3 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs_1.8.99) bash-3.2$ west init -m https://github.com/nrfconnect/sdk-nrf
=== Initializing in /Users/makmorit/opt/ncs_1.8.99
--- Cloning manifest repository from https://github.com/nrfconnect/sdk-nrf
Cloning into '/Users/makmorit/opt/ncs_1.8.99/.west/manifest-tmp'...
remote: Enumerating objects: 97976, done.
remote: Total 97976 (delta 0), reused 0 (delta 0), pack-reused 97976
Receiving objects: 100% (97976/97976), 55.01 MiB | 4.66 MiB/s, done.
Resolving deltas: 100% (72373/72373), done.
--- setting manifest.path to nrf
=== Initialized. Now run "west update" inside /Users/makmorit/opt/ncs_1.8.99.
(ncs_1.8.99) bash-3.2$ west update
=== updating zephyr (zephyr):
--- zephyr: initializing
Initialized empty Git repository in /Users/makmorit/opt/ncs_1.8.99/zephyr/.git/
--- zephyr: fetching, need revision v2.7.99-ncs1-rc1
remote: Enumerating objects: 642346, done.
:
Resolving deltas: 100% (3134/3134), done.
From https://github.com/zephyrproject-rtos/zscilib
 * [new branch]      gh-pages   -> refs/west/gh-pages
 * [new branch]      master     -> refs/west/master
 * [new tag]         v0.2.0-rc1 -> v0.2.0-rc1
HEAD is now at 12bfe3f Merge pull request #28 from microbuilder/doccleanup
HEAD is now at 12bfe3f Merge pull request #28 from microbuilder/doccleanup
(ncs_1.8.99) bash-3.2$ PATH=${PATH}:/Applications/CMake.app/Contents/bin
(ncs_1.8.99) bash-3.2$ west zephyr-export
Zephyr (/Users/makmorit/opt/ncs_1.8.99/zephyr/share/zephyr-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/Zephyr

ZephyrUnittest (/Users/makmorit/opt/ncs_1.8.99/zephyr/share/zephyrunittest-package/cmake)
has been added to the user package registry in:
~/.cmake/packages/ZephyrUnittest

(ncs_1.8.99) bash-3.2$

```

#### 依存ライブラリーの導入
Python3の依存ライブラリーをインストールします。

```
(ncs_1.8.99) bash-3.2$ pip3 install -r zephyr/scripts/requirements.txt
Ignoring windows-curses: markers 'sys_platform == "win32"' don't match your environment
Collecting pyelftools>=0.26 (from -r zephyr/scripts/requirements-base.txt (line 7))
:
Successfully installed MarkupSafe-2.0.1 Pillow-9.0.1 PyGithub-1.55 aenum-3.1.8 alabaster-0.7.12 anytree-2.8.0 appdirs-1.4.4 arrow-1.2.2 astroid-2.9.3 attrs-21.4.0 babel-2.9.1 breathe-4.32.0 canopen-1.2.1 capstone-4.0.2 cbor-1.0.0 certifi-2021.10.8 cffi-1.15.0 charset-normalizer-2.0.11 click-8.0.3 cmsis-pack-manager-0.4.0 coverage-6.3.1 cryptography-36.0.1 deprecated-1.2.13 docutils-0.18.1 future-0.18.2 gcovr-5.0 gitlint-0.17.0 gitlint-core-0.17.0 hidapi-0.11.0.post2 idna-3.3 imagesize-1.3.0 imgtool-1.8.0 importlib-metadata-4.10.1 iniconfig-1.1.1 intelhex-2.3.0 intervaltree-3.1.0 isort-5.10.1 jinja2-3.0.3 junit2html-30.0.6 junitparser-1.6.3 lazy-object-proxy-1.7.1 libusb-package-1.0.25.0 lpc-checksum-2.2.0 lxml-4.7.1 mccabe-0.6.1 milksnake-0.1.5 mock-4.0.3 mypy-0.931 mypy-extensions-0.4.3 natsort-8.1.0 platformdirs-2.4.1 pluggy-1.0.0 ply-3.11 prettytable-2.5.0 progress-1.6 protobuf-3.19.4 psutil-5.9.0 py-1.11.0 pycparser-2.21 pyelftools-0.28 pygments-2.11.2 pyjwt-2.3.0 pylink-square-0.11.1 pylint-2.12.2 pynacl-1.5.0 pyocd-0.33.0 pyocd-pemicro-1.1.3 pypemicro-0.1.9 pyserial-3.5 pytest-7.0.0 python-can-3.3.4 python-magic-0.4.25 pytz-2021.3 pyusb-1.2.1 requests-2.27.1 sh-1.14.2 snowballstemmer-2.2.0 sortedcontainers-2.4.0 sphinx-4.4.0 sphinx-notfound-page-0.8 sphinx-rtd-theme-1.0.0 sphinx-tabs-3.2.0 sphinxcontrib-applehelp-1.0.2 sphinxcontrib-devhelp-1.0.2 sphinxcontrib-htmlhelp-2.0.0 sphinxcontrib-jsmath-1.0.1 sphinxcontrib-qthelp-1.0.3 sphinxcontrib-serializinghtml-1.1.5 sphinxcontrib-svg2pdfconverter-1.2.0 tabulate-0.8.9 toml-0.10.2 tomli-2.0.0 typed-ast-1.5.2 typing-extensions-4.0.1 urllib3-1.26.8 wcwidth-0.2.5 wrapt-1.13.3 zipp-3.7.0
You are using pip version 19.0.3, however version 22.0.3 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ pip3 install -r nrf/scripts/requirements.txt
Requirement already satisfied: west>=0.12.0 in ./lib/python3.7/site-packages (from -r nrf/scripts/requirements-base.txt (line 1)) (0.12.0)
Collecting ecdsa (from -r nrf/scripts/requirements-build.txt (line 1))
:
Successfully installed CommonMark-0.9.1 azure-core-1.22.0 azure-storage-blob-12.9.0 cached-property-1.5.2 cbor2-5.4.2.post1 cddl-gen-0.3.0 ecdsa-0.17.0 isodate-0.6.1 m2r2-0.3.2 markdown-3.3.4 mistune-0.8.4 msrest-0.6.21 oauthlib-3.2.0 pygit2-1.8.0 recommonmark-0.6.0 regex-2022.1.18 requests-oauthlib-1.3.1 sphinx-markdown-tables-0.0.15 sphinx-ncs-theme-0.7.0 sphinxcontrib-mscgen-0.6 sphinxcontrib-plantuml-0.22
You are using pip version 19.0.3, however version 22.0.3 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ pip3 install -r bootloader/mcuboot/scripts/requirements.txt
Requirement already satisfied: cryptography>=2.6 in ./lib/python3.7/site-packages (from -r bootloader/mcuboot/scripts/requirements.txt (line 1)) (36.0.1)
:
Requirement already satisfied: typing-extensions>=3.6.4; python_version < "3.8" in ./lib/python3.7/site-packages (from importlib-metadata; python_version < "3.8"->click->-r bootloader/mcuboot/scripts/requirements.txt (line 3)) (4.0.1)
You are using pip version 19.0.3, however version 22.0.3 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs_1.8.99) bash-3.2$
```

#### 環境設定
`west-completion.bash`というスクリプトを生成しておきます。

```
(ncs_1.8.99) bash-3.2$ pwd
/Users/makmorit/opt/ncs_1.8.99
(ncs_1.8.99) bash-3.2$ cd zephyr
(ncs_1.8.99) bash-3.2$ west completion bash > ../west-completion.bash
(ncs_1.8.99) bash-3.2$ cd ..
(ncs_1.8.99) bash-3.2$ pwd
/Users/makmorit/opt/ncs_1.8.99
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ ls -al
total 120
drwxr-xr-x  17 makmorit  staff    544  2  7 16:42 .
:
-rw-r--r--   1 makmorit  staff  17789  2  7 16:42 west-completion.bash
drwxr-xr-x  47 makmorit  staff   1504  2  7 16:36 zephyr
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ deactivate
bash-3.2$
```

#### Matter推奨バージョンと整合

最後に、nCSのバージョンを、Matterが推奨するバージョン（タグ）と整合させます。

```
bash-3.2$ cd ${HOME}/opt/ncs_1.8.99;source bin/activate
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ export NCS_HOME=${HOME}/opt/ncs_1.8.99
(ncs_1.8.99) bash-3.2$ export ZEPHYR_BASE=${NCS_HOME}/zephyr
(ncs_1.8.99) bash-3.2$ source ${NCS_HOME}/west-completion.bash
(ncs_1.8.99) bash-3.2$
(ncs_1.8.99) bash-3.2$ /Users/makmorit/GitHub/connectedhomeip/scripts/setup/nrfconnect/update_ncs.py --update
Updating nRF Connect SDK to recommended revision...
Note: switching to 'cfedfdfa08567b2252b511a4d1db15fbeba8152d'.
:
=== updating tinycbor (modules/lib/tinycbor):
HEAD is now at 40daca9 zephyr: Remove TINYCBOR from interface libraries
=== updating tinycrypt (modules/crypto/tinycrypt):
HEAD is now at 3e9a49d cmake: Fix conditional in root CMakeLists.txt
=== updating TraceRecorderSource (modules/debug/TraceRecorder):
HEAD is now at 36c5777 Merge remote-tracking branch 'upstream/main' into zephyr
(ncs_1.8.99) bash-3.2$ deactivate
bash-3.2$
```

### サンプルアプリを使用した動作確認

nRF5340にサンプルアプリを書込み、始動まで確認します。

#### サンプルアプリのビルド

`westbuild.sh`を使用し、サンプルアプリをビルドします。

```
bash-3.2$ ./westbuild.sh
-- west build: generating a build system
Including boilerplate (Zephyr base): /Users/makmorit/opt/ncs_1.8.99/zephyr/cmake/app/boilerplate.cmake
-- Application: /Users/makmorit/GitHub/onecard-fido/MatterPoCKit/nrfconnect
-- Zephyr version: 2.7.0 (/Users/makmorit/opt/ncs_1.8.99/zephyr), build: v2.7.0-ncs1
-- Found Python3: /Users/makmorit/opt/ncs_1.8.99/bin/python3 (found suitable exact version "3.7.3") found components: Interpreter
-- Found west (found suitable version "0.12.0", minimum required is "0.7.1")
-- Board: nrf5340dk_nrf5340_cpuapp
:
[829/894] No configure step for 'chip-gn'
[830/894] Performing build step for 'chip-gn'
Generating compile_commands took 12ms
Done. Made 70 targets from 76 files in 46ms
[327/327] stamp obj/ABS_PATH/Users/makmorit/GitHub/connectedhomeip/config/nrfconnect/chip-gn/default.stamp
[885/894] Linking CXX executable zephyr/zephyr_prebuilt.elf

[892/894] Linking CXX executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:      658828 B      1008 KB     63.83%
            SRAM:      198798 B       448 KB     43.33%
        IDT_LIST:          0 GB         2 KB      0.00%
[894/894] Generating zephyr/merged_domains.hex
bash-3.2$
```

#### サンプルアプリの書込み

`westbuild.sh`を使用し、サンプルアプリをnRF5340に書込みます。

```
bash-3.2$ ./westbuild.sh -f
ZEPHYR_BASE=/Users/makmorit/opt/ncs_1.8.99/zephyr (origin: env)
-- west flash: rebuilding
cmake version 3.20.2 is OK; minimum version is 3.13.1
Running CMake: /usr/local/bin/cmake --build build
[0/9] cd /Users/makmorit/GitHub/connectedhomeip && /Users/...tup/nrfconnect/update_ncs.py --check --quiet || ( exit 0 )
:
runners.nrfjprog: nrfjprog --pinreset -f NRF53 --snr 960160943
Applying pin reset.
-- runners.nrfjprog: Board with serial number 960160943 flashed successfully.
bash-3.2$
```

#### 起動確認
nRF5340上のサンプルアプリから出力されるログを確認します。<br>
下記はどの端末ともコミッショニングを実行していない状態（＝工場出荷状態）でのログになります。

```
I: nRF5 802154 radio initialized
I: 4 Sectors of 4096 bytes
I: alloc wra: 2, f10
I: data wra: 2, 5fc
I: State changed! Flags: 0x00038200 Current role: 0
*** Booting Zephyr OS build v2.7.0-ncs1  ***
I: Init CHIP stack
I: 670 [DL]BLE address: C1:FF:E6:A6:9F:91
I: Starting CHIP task
I: Init Thread stack
I: 726 [DL]OpenThread started: OK
I: 729 [DL]Setting OpenThread device type to MINIMAL END DEVICE
I: 735 [ZCL]Using ZAP configuration...
D: 750 [DMG]Failed to read stored attribute (0, 0x0000_0028, 0x0000_0005: a0
D: 767 [DMG]Failed to read stored attribute (0, 0x0000_0028, 0x0000_0010: a0
D: 784 [DMG]Failed to read stored attribute (0, 0x0000_002B, 0x0000_0001: a0
D: 802 [DMG]Failed to read stored attribute (0, 0x0000_002C, 0x0000_0000: a0
D: 819 [DMG]Failed to read stored attribute (0, 0x0000_002C, 0x0000_0001: a0
I: 828 [ZCL]Initiating Admin Commissioning cluster.
I: 833 [ZCL]OpCreds: Initiating OpCreds cluster by writing fabrics list from fabric table.
D: 841 [DIS]Add fabric pairing table delegate
E: 845 [ZCL]Trying to write invalid Calendar Type
E: 850 [ZCL]Failed to write calendar type with error: 0x87
D: 855 [DMG]Endpoint 1, Cluster 0x0000_0006 update version to f11c8dcb
D: 861 [DIS]Init fabric pairing table with server storage
D: 920 [DMG]AccessControl: initializing
D: 923 [DMG]Examples::AccessControlDelegate::Init
D: 938 [DMG]AccessControl: unable to load stored ACL entries; using empty list instead
D: 946 [IN]UDP::Init bind&listen port=5540
D: 950 [IN]UDP::Init bound to port=5540
D: 953 [IN]TransportMgr initialized
D: 978 [DIS]DNS-SD StartServer modeHasValue=0 modeValue=0
D: 983 [DL]Using Thread extended MAC for hostname.
I: 988 [DIS]Failed to find a valid admin pairing. Node ID unknown
I: 994 [DIS]Start dns-sd server - no current nodeId
D: 998 [DL]Using Thread extended MAC for hostname.
I: 1014 [DIS]Advertise commission parameter vendorID=9050 productID=20043 discriminator=3840/15
E: 1022 [DIS]Failed to advertise unprovisioned commissionable node: Error CHIP:0x00000003
D: 1030 [DIS]Scheduling Discovery timeout in secs=900
E: 1035 [DIS]Failed to finalize service update: Error CHIP:0x0000001C
I: 1041 [IN]CASE Server enabling CASE session setups
D: 1046 [DL]Using Thread extended MAC for hostname.
I: 1051 [SVR]Server Listening...
I: 1054 [DL]Device Configuration:
I: 1067 [DL]  Serial Number: TEST_SN
I: 1070 [DL]  Vendor Id: 9050 (0x235A)
I: 1074 [DL]  Product Id: 20043 (0x4E4B)
I: 1088 [DL]  Hardware Version: 0
I: 1101 [DL]  Setup Pin Code: 20202021
I: 1115 [DL]  Setup Discriminator: 3840 (0xF00)
I: 1130 [DL]  Manufacturing Date: (not set)
I: 1134 [DL]  Device Type: 65535 (0xFFFF)
I: 1159 [SVR]SetupQRCode: [MT:W0GU2OTB00KA0648G00]
I: 1163 [SVR]Copy/paste the below URL in a browser to see the QR Code:
    https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3AW0GU2OTB00KA0648G00
I: 1198 [SVR]Manual pairing code: [34970112332]
I: 1224 [SVR]Long manual pairing code: [749701123309050200434]
D: 1229 [DL]CHIP task running
I: 1243 [ZCL]GeneralDiagnosticsDelegate: OnDeviceRebooted
D: 1248 [DMG]Endpoint 0, Cluster 0x0000_0033 update version to b318c38c
I: 1254 [ZCL]PlatformMgrDelegate: OnStartUp
D: 1258 [EVL]LogEvent event number: 0x0000000000000000 priority: 2, endpoint id:  0x0 cluster id: 0x0000_0028 event id: 0x0 Sys timestamp: 0x00000000000004EA
```
