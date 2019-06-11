# nRF52840版 FIDO2.0認証器

## 概要
FIDO U2F／WebAuthn（CTAP2）の仕様に準拠したUSB HID／BLEアプリケーションです。

- PCにおけるユーザー登録／ログイン実行時[注1]：nRF52840 DongleをUSBポートに挿すと、nRF52840のUSB HIDサービスを経由して処理が行われます。

- Androidにおけるログイン実行時[注1]：nRF52840 DKの「nRF USB端子」をUSBポートに接続しない状態で使用すると、nRF52840のBLEサービスを経由してログイン処理が行われます。[注2]

[注1] U2Fではユーザー登録＝Register、ログイン＝Authenticate、WebAuthnではユーザー登録＝MakeCredential、ログイン＝GetAssertionと、実行されるコマンドが異なります。<br>
[注2] 2019/05/07現在、Chrome Android上ではU2F Authenticateコマンドのみのサポートとなっているようです。

FIDO U2F／CTAP2に関する情報 : https://fidoalliance.org/download/

### ハードウェア

主として、nRF52840 Dongle＋PCのUSBポートの組み合わせによるWebAuthn操作を想定しています。[注1]

ただし、nRF52840 DK（開発用ボード）でも動作させることが可能です。[注2]<br>
この場合は、J-Link（プログラミングで使用するUSBポート）経由のUARTにより、デバッグログを表示させることができます。

[注1] 常にUSB HIDデバイスとして動作しますが、BLEセントラルデバイスとしても稼働できるよう実装されています。<br>
[注2] nRF USB端子をUSBケーブル経由でPCに接続した場合はUSB HIDデバイス、未接続の場合はBLEペリフェラルデバイスとして動作します（プログラムにより自動的に切り替えが行われます）。<br>


### ファームウェア

「nRF52840版 FIDO2.0認証器」で使用するファームウェアは下記の２本になります。<br>
フォルダー[`firmwares`](firmwares/README.md) に格納しています。

- s140_nrf52_6.1.1_softdevice.hex - ソフトデバイス

- nrf52840_DK_xxaa.hex - アプリケーション（nRF52840 DK用）

- nrf52840_DG_xxaa.hex - アプリケーション（nRF52840 Dongle用）

これらのファームウェアは、Nordic社から提供されているアプリ「nRF Connect」を使い、nRF52840 DongleやnRF52840 DKにダウンロードするようにします。<br>
「nRF Connect」を使用したダウンロード手順は、別ドキュメント<b>「[nRF52840 Dongleプログラミング手順](../Development/nRF52840/NRFCONNECTINST.md)」</b>をご参照願います。
　
## 動作確認手順

### WebAuthn

nRF52840 Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのMakeCredential／GetAssertionの動作確認をする場合は、以下の手順で進めるようにします。

- <b>[Edgeブラウザーを使用したWebAuthnテスト手順](../Research/FIDO_2_0/EDGETEST.md)</b>

### U2F

nRF52840 Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合は、以下の手順で進めるようにします。

- <b>[PCでのU2F Register/Authenticate確認手順](../nRF5_SDK_v15.2.0/PCCHROME.md)</b>

- <b>[AndroidでのU2F Authenticate確認手順](../nRF5_SDK_v15.2.0/ANDROID.md)</b>

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[NetBeansインストール手順](NETBEANSINST.md)</b>
