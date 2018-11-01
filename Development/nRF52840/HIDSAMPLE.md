# [WIP] USB HIDサンプルアプリ動作確認手順

サンプルアプリ[`USB HID Generic Example`](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.2.0/usbd_hid_generic_example.html?cp=4_0_0_4_5_50_6)を使用して、nRF52840のUSB HID機能を確認する手順を掲載しています。

## 機材の準備

USB HIDデバイスとなるnRF52840を準備します。
今回の検証では、[nRF52840 DK](https://www.mouser.jp/new/nordicsemiconductor/nordic-nrf52840-dev-kit/)という開発ボードを使用します。

<img src="assets/0033.png" width="500">

またPCと接続するため、USBケーブル（Type micro-B）を２本用意します。

## サンプルアプリケーションの準備

サンプルアプリ[`USB HID Generic Example`](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.2.0/usbd_hid_generic_example.html?cp=4_0_0_4_5_50_6)を、[NetBeans](NETBEANS.md)によりビルドし、nRF52840 DKに書込みます。

### サンプルアプリソースのコピー／配置

別途手順「[NetBeans開発環境構築手順](NETBEANS.md)」で、NetBeansとnRF5 SDK（v15.2）を導入したら、以下の場所にあるサンプルアプリを、フォルダーごと任意の場所にコピーします。

サンプルアプリの場所：<br>
`~/opt/nRF5_SDK_15.2.0/examples/peripheral/usbd_hid_generic`

コピー先は以下の場所としておきます。<br>
`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/usb/usbd_hid_generic`

### Makefileの修正

ここで、サンプルアプリの場所（`~/opt/nRF5_SDK_15.2.0/`）と、コピー先の場所（`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/`）が異なるため、Makefileを事前に修正する必要があります。<br>
（修正を忘れると、プロジェクト新規作成後の自動ビルドに失敗してしまうための事前措置になります）

`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/usb/usbd_hid_generic/pca10056/blank/armgcc`配下のMakefileというファイルをエディターで開き、<br>
`SDK_ROOT := ../../../../../..`<br>とあるのを<br>`SDK_ROOT := $(HOME)/opt/nRF5_SDK_15.2.0`<br>と修正してください。

<img src="assets_hid/0001.png" width="500">
