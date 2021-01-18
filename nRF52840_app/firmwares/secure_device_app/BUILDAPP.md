# nRF52840アプリケーション作成手順書

Nordic社サンプルアプリケーションを土台に、nRF52840アプリケーションを作成する手順を記載しています。

## nRF52840アプリケーションについて

[Nordic社提供のサンプルアプリケーション](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/ble_sdk_app_hrs.html)をベースに、BLEセントラル／USBインターフェースを増設した上で、業務アプリケーション（FIDO2機能、PIV機能、メンテナンス機能等）を搭載しています。


## 事前準備

#### NetBeans環境の作成

あらかじめ、NetBeans環境をPCに作成しておきます。<br>
具体的な手順は、[NetBeansインストール手順](../../../nRF52840_app/NETBEANSINST.md)をご参照ください。

#### nRF Utilのインストール

ビルドを実行する際に必要となる、nRF UtilをPCにインストールしておきます。<br>
具体的な手順は、[nRF Utilインストール手順](../../../nRF52840_app/NRFUTILINST.md)をご参照ください。

本手順書を作成した時点でのnRF Utilは、`version 6.1`となっておりました。

```
bash-3.2$ nrfutil version
nrfutil version 6.1.0
bash-3.2$
```

#### MDBT50Q Dongleの初期化

[USBブートローダー書込み手順書](../../../nRF52840_app/firmwares/secure_bootloader/WRITESBL.md)の手順により、初期化されたMDBT50Q Dongleを準備します。

## ソースファイルの準備

#### オリジナルソースの取得

nRF5 SDKのサンプルアプリケーション・フォルダー（`${HOME}/opt/nRF5_SDK_17.0.2/examples/ble_peripheral/ble_app_hrs`）から、必要なソースコードを取得します。<br>
今回の作成にあたっては、[`<リポジトリールート>/nRF52840_app/examples/diverta`](../../../nRF52840_app/examples/diverta)配下に配置いたしました。

`ble_app_hrs`サブフォルダーには、不要なファイルが含まれていますので、適宜削除します。<br>
具体的には下記イメージになるかと存じます。

<img src="assets01/0001.jpg" width="400">

その後、ファイル接頭辞`ble_app_hrs`を、`secure_device_app`にリネームします。<br>
リネームしたのちのイメージは下図の通りです。

<img src="assets01/0002.jpg" width="400">

#### 秘密鍵ファイルの存在確認

[USBブートローダーファームウェア](../../../nRF52840_app/firmwares/secure_bootloader/BUILDSBL.md)生成時に使用した公開鍵に対応する秘密鍵ファイル（`secure_bootloader_prvkey.pem`）を、ユーザールートディレクトリーに配置しておきます。<br>
このファイルがないと、サンプルアプリケーションをUSBポート経由で書込みできません。

```
bash-3.2$ cd ${HOME}
bash-3.2$ ls -al *.pem
-rw-r--r--  1 makmorit  staff  227  7  8  2020 secure_bootloader_prvkey.pem
bash-3.2$
```

#### SDKソースコードの修正

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)は、Nordic社のドングル「nRF52840 Dongle」から回路を修正しているため、`PCA_10059`のソースがそのまま利用できません。<br>
また、FIDO2機能で使用する汎用USB HIDサービスは、Nordic社ライブラリーの内容では動作させることができません。<br>
そのため、nRF5 SDKのフォルダー（`/nRF5_SDK_17.0.2/components/`）から、必要なソースコードを取得し、一部修正を加えております。

MDBT50Q Dongle用の独自定義、独自実装は下記ファイルになります。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`pca10059_01.h`|[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ヘッダーファイル|
|2|`pca10059_02.h`|[MDBT50Q Dongle（rev2.1.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ヘッダーファイル|
|3|`app_usbd_core.c`|USBサービス実装を実装（オリジナルから一部修正）|
|4|`app_usbd_hid_generic.c`|USB HIDサービス実装を実装（オリジナルから一部修正）|

今回の作成にあたっては、[`<リポジトリールート>/nRF52840_app/components`](../../../nRF52840_app/components)配下に配置いたしました。

#### メイクファイルの修正

メイクファイル「[Makefile](../../../nRF52840_app/examples/diverta/secure_device_app/pca10056/s140/armgcc/Makefile)」について、オリジナルから修正を加えます。

#### define追加

【追加した行】
```
# target board
#  PCA10059_01  MDBT50Q Dongle(rev2, without ATECC608A)
#  PCA10059_02  MDBT50Q Dongle(rev2.1.2, with ATECC608A)
TARGET_BOARD     := PCA10059_01

# application version info
FW_REV := 0.2.13
CFLAGS += -DFW_REV=\"$(FW_REV)\"
# hardware version info
CFLAGS += -DHW_REV=\"$(TARGET_BOARD)\"

# enable log output on PCA10056
ifeq ("$(TARGET_BOARD)","PCA10056")
    CFLAGS += -DNRF_LOG_BACKEND_UART_ENABLED=1
else
    CFLAGS += -DNRF_LOG_BACKEND_UART_ENABLED=0
endif

# for mbedtls_rsa_private
CFLAGS += -DMEMORY_MANAGER_XLARGE_BLOCK_COUNT=64

# disable ATECC608A (or ATECC608A is not installed)
# CFLAGS += -DNO_SECURE_IC
```

[注1] `TARGET_BOARD`の定義は、ブートローダー導入先の基板名を指定します。`PCA10059_01`を指定すると、MDBT50Q Dongle(rev2)向けのファームウェア更新イメージが生成されます。

#### パス修正

【修正前】
```
SDK_ROOT := ../../../../../..
PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := ble_app_hrs_gcc_nrf52.ld
```

【修正後】<br>
nRF52840アプリケーションに必要な定義を適宜加えています。<br>
また、リンク用スクリプトファイル名を変えています。
```
SDK_ROOT := $(HOME)/opt/nRF5_SDK_17.0.2
PROJ_DIR := ../../..
FIDO_DIR := $(PROJ_DIR)/../../../../FIDO2Device
CCID_DIR := $(PROJ_DIR)/../../../../CCID
FD2LIB_DIR := $(FIDO_DIR)/fido2_lib
U2FLIB_DIR := $(FIDO_DIR)/u2f_lib
CT2LIB_DIR := $(FIDO_DIR)/ctap2_lib
CIDLIB_DIR := $(CCID_DIR)/ccid_lib
PLTLIB_DIR := $(PROJ_DIR)/../plat_lib
BLELIB_DIR := $(PROJ_DIR)/../ble_lib
DMOLIB_DIR := $(PROJ_DIR)/../demo_lib
SDK_CUSTOM_ROOT := $(PROJ_DIR)/../../..
DEPLOY_ROOT := $(SDK_CUSTOM_ROOT)/firmwares

# tinycbor root directory
TINYCBOR_ROOT := $(HOME)/GitHub/tinycbor

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := secure_device_app_gcc_nrf52.ld
```

#### ターゲット変更（４箇所あります）

【修正前】
```
$(SDK_ROOT)/components/boards/boards.c \
：
$(SDK_ROOT)/components/boards \
：
CFLAGS += -DBOARD_PCA10056
：
ASMFLAGS += -DBOARD_PCA10056
：
default: nrf52840_xxaa
```

【修正後】
```
$(SDK_CUSTOM_ROOT)/components/boards/boards.c \
：
$(SDK_CUSTOM_ROOT)/components/boards \
：
CFLAGS += -DBOARD_$(TARGET_BOARD)
：
ASMFLAGS += -DBOARD_$(TARGET_BOARD)
：
ifeq ("$(TARGET_BOARD)","PCA10056")
    default: $(TARGETS) deploy_dk
else
    default: $(TARGETS) secure_pkg
endif
```

#### Oberonの削除

【削除した行】<br>
ビルド対象コードから、Oberon関連のソースコードを削除しました。
```
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_chacha_poly_aead.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecc.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecdh.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecdsa.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_eddsa.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_hash.c \
$(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_hmac.c \
```

#### リンク先ファイルの修正

FIDO2機能で必要なtinyCBORのライブラリーを追加でリンクします。<br>
また、Oberonは不要なので削除しています。

【修正前】
```
LIB_FILES += \
  $(SDK_ROOT)/external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.13.a \
  $(SDK_ROOT)/external/nrf_oberon/lib/cortex-m4/hard-float/liboberon_3.0.6.a \
```

【修正後】
```
# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.13.a \
  $(TINYCBOR_ROOT)/lib/libtinycbor.a \
```

#### デプロイ処理追加

【追加した行】
```
deploy_dk:
	@echo Application hex file for PCA10056 is now available.

secure_pkg:
	/usr/local/bin/nrfutil pkg generate --hw-version 52 --sd-req 0x0100 --application-version-string $(FW_REV) --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --key-file $(HOME)/secure_bootloader_prvkey.pem $(DEPLOY_ROOT)/appkg.$(TARGET_BOARD).$(FW_REV).zip
	@echo Application zip package for secure bootloader is now available.
```

## ソースファイルからビルド

上記で取得したソースファイルから、NetBeansプロジェクトを新規作成し、動作確認用のファームウェア更新イメージ（`appkg.<基板名>.<バージョン文字列>.zip`ファイル）を生成します。<br>
下記例では、ファームウェア更新イメージファイル名は`appkg.PCA10059_01.0.2.13.zip`となります。

#### プロジェクトの新規作成〜ビルド実行

NetBeansを起動し、ファイル--->新規プロジェクトを実行します。

<img src="assets01/0003.jpg" width="300">

新規プロジェクト画面が表示されますので、一覧から「既存のソースを使用するC/C++プロジェクト」を選択し「次 >」をクリックします。

<img src="assets01/0004.jpg" width="350">

下図のような画面に遷移しますので、以下のように設定します。

- 既存のソースを含むフォルダを指定 - サンプルアプリが格納されているフォルダー「`examples/diverta/secure_device_app`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF52840_app/examples/diverta/secure_device_app`」という文字列が設定されています。

- ツール・コレクションを選択 - 「GNU_ARM (GNU Mac)」をチェックします。
- 構成モードを選択 - 「カスタム(C)」をチェックします。

設定が完了したら「次 >」をクリックします。

<img src="assets01/0005.jpg" width="350">

下図のような画面に遷移しますので、以下のように設定します。

- 「事前ビルド・ステップが必要」にチェック

- フォルダで実行(U) - サンプルアプリのサブフォルダー「`pca10056/s140/armgcc`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF52840_app/nRF52840_app/examples/diverta/secure_device_app/pca10056/s140/armgcc`」という文字列が設定されています。

- 「カスタム・コマンド」にチェック

- コマンド(O) - 「make」と入力します。

設定が完了したら「次 >」をクリックします。

<img src="assets01/0006.jpg" width="350">

「4. ビルド・アクション」に遷移しますが、以降は「7. プロジェクトの名前と場所」に遷移するまではデフォルト設定のまま「次 >」をクリックします。

<img src="assets01/0007.jpg" width="350">

「7. プロジェクトの名前と場所」に遷移したら、プロジェクト名(P)を「`secure_device_app`」から「`secure_device_app_proj`」に変更しておきます。<br>
（ソースフォルダー名「`secure_device_app`」を上書きしたくないための措置です）

設定が完了したら「終了(F)」をクリックします。

<img src="assets01/0008.jpg" width="350">

自動的にビルドがスタートしますので、しばらくそのまま待ちます。<br>
しばらくするとビルドが完了し「ビルド SUCCESSFUL」と表示されれば、ビルドは成功です。

<img src="assets01/0009.jpg" width="600">

#### ビルド結果の確認

ビルドが完了したら、ファームウェア更新イメージファイル`appkg.PCA10059_01.0.2.13.zip`が正しく生成されているかどうか確認します。<br>
下記は、ターミナルで`appkg.PCA10059_01.0.2.13.zip`(109KB)が生成されたことを確認したところです。

```
bash-3.2$ cd ${HOME}/GitHub/onecard-fido/nRF52840_app/firmwares/
bash-3.2$ ls -al *.zip
-rw-r--r--  1 makmorit  staff  108905  1 18 09:38 appkg.PCA10059_01.0.2.13.zip
bash-3.2$
```

以上で、ソースファイルからのビルドは完了です。

#### 動作確認

この時点では、ファームウェアはまだ、Nordic社提供のサンプル「[Heart Rate Application](../../../nRF52840_app/firmwares/sample_blehrs/BUILDHRS.md)」と同等の機能になっています。<br>
したがって、別途手順書「[サンプルアプリケーション動作確認手順書](../../../nRF52840_app/firmwares/sample_blehrs/WRITEHRS.md)」どおりに動作確認を進めます。

動作確認の結果がOKであれば、前SDKバージョンのnRF52840アプリケーションから、ソースコードの移行作業を進めていくことになります。
