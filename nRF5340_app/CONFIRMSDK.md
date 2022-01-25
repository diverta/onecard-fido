# nRF Connect SDK動作確認手順書

macOSにインストールされた「[nRF Connect SDK v1.8.0](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/)」の動作確認手順について掲載します。

## 手順の概要

- <b>ソフトウェアのインストール</b><br>
本手順書で必要となる各種ソフトウェアを、macOSにインストールします。

- <b>サンプルアプリのビルド／書込み</b><br>
Nordic社から公開されているサンプルアプリ「[Peripheral UART](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/samples/bluetooth/peripheral_uart/README.html#bluetooth-peripheral-uart)」を、nRF Connect SDKでビルドし、nRF5340に書込みます。

- <b>サンプルアプリの動作確認</b><br>
Androidアプリ「nRF Connect」を使用し、nRF5340に書き込んだ「Peripheral UART」が正常に動作することを確認します。

## ソフトウェアのインストール

本手順書で必要となるソフトウェア「nRF Command Line Tools」を、macOSにインストールします。<br>
具体的な手順につきましては、別途手順書<b>「[nRF Command Line Toolsインストール手順](../nRF52840_app/NRFCLTOOLINST.md)」</b>をご参照願います。

## サンプルアプリのビルド／書込み

Nordic社から公開されているサンプルアプリ「[Peripheral UART](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/samples/bluetooth/peripheral_uart/README.html#bluetooth-peripheral-uart)」を、nRF Connect SDKでビルドし、nRF5340に書込みます。

### サンプルアプリのコピー

nRF Connect SDKのサンプルアプリを、適宜フォルダーにコピーします。

```
bash-3.2$ cd ${HOME}/opt/ncs_1.8.0-rc2;source bin/activate
(ncs_1.8.0-rc2) bash-3.2$ cd ${HOME}/GitHub/onecard-fido/nRF5340_app/
(ncs_1.8.0-rc2) bash-3.2$ cp -pr ${HOME}/opt/ncs_1.8.0-rc2/nrf/samples/bluetooth/peripheral_uart .
(ncs_1.8.0-rc2) bash-3.2$ ls -al
total 312
drwxr-xr-x  20 makmorit  staff    640  1 25 16:12 .
drwxr-xr-x  19 makmorit  staff    608  1 25 09:14 ..
:
drwxr-xr-x  13 makmorit  staff    416  1 25 10:04 peripheral_uart
:
(ncs_1.8.0-rc2) bash-3.2$
```

### ビルド用スクリプトを配置

ビルド用スクリプト`westbuild.sh`を作成し、プロジェクトフォルダー配下に配置します。

```
(ncs_1.8.0-rc2) bash-3.2$ cd ${HOME}/GitHub/onecard-fido/nRF5340_app/peripheral_uart
(ncs_1.8.0-rc2) bash-3.2$ ls -al
total 80
drwxr-xr-x  14 makmorit  staff   448  1 25 16:14 .
drwxr-xr-x  20 makmorit  staff   640  1 25 16:12 ..
:
-rwxr-xr-x@  1 makmorit  staff   949  1 25 16:14 westbuild.sh
(ncs_1.8.0-rc2) bash-3.2$

```

内容は以下になります。

```
#!/bin/bash

# Build target
#   nrf5340dk_nrf5340_cpuapp
export BUILD_TARGET=nrf5340dk_nrf5340_cpuapp

# Environment variables for the GNU Arm Embedded toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH="${HOME}/opt/gcc-arm-none-eabi-9-2020-q2-update"

# Paths for command
export PATH=${PATH}:/Applications/CMake.app/Contents/bin

# bash completion
export NCS_HOME=${HOME}/opt/ncs_1.8.0-rc2
export ZEPHYR_BASE=${NCS_HOME}/zephyr
source ${NCS_HOME}/west-completion.bash

# Enter Python3 venv
source ${NCS_HOME}/bin/activate

if [ "$1" == "-f" ]; then
    # Flash for nRF5340
    ${NCS_HOME}/bin/west -v flash -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
else
    # Build for nRF5340
    rm -rf build_signed
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed
    if [ `echo $?` -ne 0 ]; then
        deactivate
        exit 1
    fi
fi

deactivate
exit 0
```

### ビルド実行

ビルド用スクリプト`westbuild.sh`を実行し、プロジェクトをビルド（コンパイル、リンク）します。<br>
（実行時のログ`westbuild.log`は<b>[こちら](assets01/westbuild.log)</b>）

```
(ncs_1.8.0-rc2) bash-3.2$ pwd
/Users/makmorit/GitHub/onecard-fido/nRF5340_app/peripheral_uart
(ncs_1.8.0-rc2) bash-3.2$ ./westbuild.sh > westbuild.log 2>&1
(ncs_1.8.0-rc2) bash-3.2$ echo $?
0
(ncs_1.8.0-rc2) bash-3.2$
```


### 書込み

ビルド用スクリプト`westbuild.sh -f`を実行し、ビルドしたファームウェアを、nRF5340に書込みます。<br>
（実行時のログ`westbuild.log`は<b>[こちら](assets01/westbuild_f.log)</b>）

```
(ncs_1.8.0-rc2) bash-3.2$ pwd
/Users/makmorit/GitHub/onecard-fido/nRF5340_app/peripheral_uart
(ncs_1.8.0-rc2) bash-3.2$ ./westbuild.sh -f > westbuild_f.log 2>&1
(ncs_1.8.0-rc2) bash-3.2$ echo $?
0
(ncs_1.8.0-rc2) bash-3.2$
```

### ファームウェア起動確認

nRF5340開発ボード（`PCA10095`）に接続し、`screen`コマンドでデバッグプリントを監視すると、ファームウェア書込み完了後に以下のようなログが出力されます。

```
*** Booting Zephyr OS build v2.7.0-ncs1-rc2  ***
Starting Nordic UART service example
```

## サンプルアプリの動作確認

Androidアプリ「nRF Connect」を使用し、nRF5340に書き込んだ「Peripheral UART」が正常に動作することを確認します。

### 接続とデータ送受信

Androidアプリ「nRF Connect」を起動します。<br>
デバイス一覧に「Nordic_UART_Service」がリストされていることを確認します。

その後、右横の「CONNECT」ボタンをタップします。

<img width="160" src="assets01/0011.jpg">

画面右上のメニューを表示させ、「Discover services」をタップします。

<img width="160" src="assets01/0012.jpg">

サービス一覧が表示されます。<br>
一覧の中から「Nordic UART Service」をタップします。

<img width="160" src="assets01/0013.jpg">

下部に３点のキャラクタリスティックが一覧表示されます。

「TX Charactaristic」の右横のアイコンをタップすると、DescriptorsのValueが「Notifications enabled」に切り替わります。<br>
この状態で、nRF5340から文字列データを受信することができるようになります。

まずは、AndroidからnRF5340へ、文字列データを送信してみます。<br>
「RX Charactaristic」の右横のアイコンをタップします。

<img width="160" src="assets01/0014.jpg">

下図のようなポップアップが表示されるので、任意の文字列（`qwerty`）を入力し「SEND」をタップします。

<img width="160" src="assets01/0015.jpg">

Android側から送信した文字列データが、nRF5340側で受信されます。<br>
下図のように、受信した文字列データ（`qwerty`）がデバッグ出力されます。

次に、nRF5340からAndroidへ、文字列データを送信してみます。<br>
screenコマンドが実行中のターミナル画面上で、任意の文字列（`asdfg`）を入力し、Enterキーを押します。<br>
（入力した`asdfg`は、ターミナル画面上にエコーバックされないのでご注意ください）

<img width="500" src="assets01/0016.jpg">

nRF5340側から送信した文字列データが、Android側で受信されます。<br>
「TX Charactaristic」のValueに、受信した文字列データ（`asdfg`）が表示されます。

<img width="160" src="assets01/0017.jpg">

以上で、サンプルアプリの動作確認は完了です。
