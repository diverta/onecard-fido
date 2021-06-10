# Matterに関する調査

新しいスマートホーム共通規格「Matter」についての調査になります。

## 概要

#### [Matterについて](../../Research/Matter/MATTERDESC.md)
新しいスマートホーム共通規格「Matter」についての概要を記述しています。<br>
下記リンクの翻訳からまとめたドキュメントになります。<br>
[GitHub - project-chip/connectedhomeip](https://github.com/project-chip/connectedhomeip)

## サンプルに関する調査

Zigbee Allianceが、Threadネットワーク内で稼働するLock（施錠）アプリのサンプルを公開しているようです。<br>
この中に、nRF52840 Lockアプリというサンプルがあるので、手始めにこのサンプルアプリについて調査します。

## nRF52840 Lockアプリ

下記リンクの翻訳になります。<br>
[https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/nrfconnect](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/nrfconnect)

文中の`CHIP`という文言は、適宜`Matter`と読み替えていただければ幸いです。<br>
（以下、本ドキュメントについて同様です）

#### [nRF52840 Lockサンプルアプリの概要](../../Research/Matter/LOCKAPPSAMPLE.md)

nRF52840を使用し、Threadネットワーク内で稼働するLock（施錠）サンプルアプリの概要です。

#### [nRF52840 Lockサンプルアプリ導入手順](../../Research/Matter/LOCKAPPSAMPLEBLD.md)

サンプルアプリ[`CHIP nRF Connect Lock Example Application`](https://github.com/project-chip/connectedhomeip/blob/master/examples/lock-app/nrfconnect/README.md)をビルドし、nRF52840に書込みを行う手順を記載しています。

### [nRF Connect CHIPプラットフォームについて](../../Research/Matter/CHIPPLATFORM.md)

nRF5340／nRF52840の標準プラットフォームである「nRF Connect SDK」をベースとし、Matterアプリケーションを実装するプラットフォームモデルです。<br>
下記リンクの翻訳になります。<br>
[https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_platform_overview.md](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_platform_overview.md)

## CHIPネットワークについての調査

#### [OpenThread Dongle導入手順](../../Research/Matter/OTDONGLE.md)

[MDBT50Q Dongle](https://github.com/diverta/onecard-fido/tree/master/FIDO2Device/MDBT50Q_Dongle)を`OpenThread Dongle`として使用する手順について記載しています。

#### [OpenThread Border Router導入手順](../../Research/Matter/OTBRSETUP.md)

Threadネットワーク〜インターネット間のルーター役となる「[`OpenThread Border Router`](https://openthread.io/guides/border-router)」の構築手順を記載しています。

#### [CHIPネットワーク設定手順](../../Research/Matter/CHIPNWSETTING.md)

前述のOpenThread Border Router、OpenThread Dongleを使用し、CHIPネットワークを設定する手順について掲載しています。

## ご参考

#### [OpenThread Commissioner導入手順](../../Research/Matter/OTCOMMSETUP.md)

`OpenThread Commissioner`は、Thread Routerデバイスを、Thread Leaderデバイスの管理下に配備（Join）させるために必要となる設定用アプリケーションです。<br>
この`OpenThread Commissioner`を、Raspberry Pi 3にインストールする手順について記載しています。
