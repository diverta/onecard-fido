# 開発TODO

現在進行中、もしくは将来予定している開発案件についての概要を掲載します。

## 各種アプリケーション改善対応

#### 要件確定済み

- <b>[調査] nRF Connect SDKへの移植に関する調査</b>
（[#394](https://github.com/diverta/onecard-fido/issues/394) ご参照）
  - 現状のnRF52840をベースとした基板のアプリケーションを、nRF Connect SDKに移植
  - 移植後、問題が無いようであれば、次世代SoCである「nRF5340」への移植を実施
  - 詳細はドキュメント「<b>[nRF Connect SDKに関する調査](Research/nRFCnctSDK_v1.4.99/README.md)</b>」ご参照

#### 要件未確定

- [改善] nRF52840アプリケーションのソースコードを圧縮する<br>
[nRF52840アプリケーション](nRF5_SDK_v15.3.0)から、不要コードを削除／重複実装コードを整理／etc...

- [改善] 管理ツールから「開発者向けツール」「エンドユーザー向けツール」を分割<br>
  - BLEペアリング・ファームウェア更新・ヘルスチェック・PIN変更といった、エンドユーザーが利用する機能に限定。<br>
  - PIN初期設定、鍵・証明書インストール／削除といった機能は、ベンダーが行うものなので、エンドユーザーには公開しないようにする<br>
  - 開発者に必要な機能を追加（USBのVID／PID変更、特定バージョンのファームウェアを任意選択／更新する機能、etc...）

## [CCIDインターフェース追加対応](CCID/README.md)
PIV Card、OpenPGP Cardなどといったスマートカードのエミュレーションに必要な基盤技術を確立します。<br>
（[#323](https://github.com/diverta/onecard-fido/issues/323) ご参照）

#### 完了済み作業
- CCIDインターフェース本体の、nRF52840アプリケーションへの追加実装
- CCIDインターフェース本体の、macOS環境／Windows10環境上での実機動作確認
- 「Yubico PIV Tool」による初期データ導入機能
- 業務アプリケーション（PIV）の開発<br>
（PIVカードエミュレーション。RSA-2048秘密鍵／証明書による認証機能を含む）
- 管理ツールによる初期データ導入機能<br>
（PIN設定変更機能を含む）

#### 今後必要な対応
- 業務アプリケーション（OpenPGP）の開発

#### 各種手順書
- [PIN番号を使用したmacOSログイン確認手順](FIDO2Device/MDBT50Q_Dongle/PIVPINLOGIN.md)

#### 各種調査結果
- [CCIDインターフェースに関する調査](Research/CCID/README.md)

## ワンタイムパスワード対応
TOTPの実装に必須となる「RTCC（リアルタイムクロック・カレンダー）」の追加実装可能性について調査します。<br>
（[#329](https://github.com/diverta/onecard-fido/issues/329) ご参照）

#### 今後必要な対応
- HID Keyboard I/Fの追加実装（前述・スマートカードエミュレート機能でも必要になる可能性あり）
- 業務アプリケーション（OATH-TOTP）の開発
- RTCC評価用の基板製作（I2Cコマンド検証用／バックアップ機能検証用／etc...）

## [セキュアIC組込み対応](SECUREIC/README.md)
完了しております。

## その他
優先度は低くなります

#### MDBT50Q Dongleの極小化対応
MDBT50Q Dongleについて、基板実装要件が全て出揃ったところで、基板をさらに極小化する試みになります。<br>
（[#280](https://github.com/diverta/onecard-fido/issues/280) ご参照）

#### CTAP 2.1対応

[FIDO認証器の新しい仕様（CTAP 2.1）](https://fidoalliance.org/specs/fido2/fido-client-to-authenticator-protocol-v2.1-rd-20191217.html)がプレビュー公開されております。<br>
現行の仕様はこちら --> [CTAP 2.0](https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html)

調査（[#321](https://github.com/diverta/onecard-fido/issues/321) ご参照）の結果、CTAP 2.1で追加されたコマンド`authenticatorSelection`については、追加対応が必要であることが判明しています。<br>
調査結果はこちら --> [CTAP 2.1 関連調査](https://github.com/diverta/onecard-fido/blob/research-CTAP2.1-new-spec/Research/CTAP_2_1/README.md)

ただし、2020/04/23現在、FIDOアライアンスからFIDO2.1に関する仕様適合テストツールが公開されていないため、実装作業は後日改めて再開したいと考えます。

#### NFC対応（中断中）

自己発電機能のあるNFCチップ「[ST25DV](https://www.st.com/ja/nfc/st25dv-i2c-series-dynamic-nfc-tags.html)」と、前出のセキュアIC「ATECC608A」を活用した、電池不要の高機能セキュリティーデバイスを製作する試みになります。

#### MBED OS移植対応（中断中）

「NUCLEO-F411RE」を使用し、MBED OS用に移植作業を進めていましたが、以下の問題点があり、現在開発を中断しています。
- 搭載されている「STM32F411RE」に、乱数製造器（RNG）の実装がないため、暗号化関連ライブラリー（MBED TLS）が使用できない
- 予定していたNFCタグ「AS3956」が製造中止-->調達不可

移植作業中のアプリケーションは以下の場所に格納しています。<br>
コード格納場所--->[[WIP] mbed OS版 FIDO2.0認証器](STM32F411RE)
