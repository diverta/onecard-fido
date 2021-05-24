# Matterに関する調査

新しいスマートホーム共通規格「Matter」についての調査になります。

## サンプルに関する調査

Zigbee Allianceが、Threadネットワーク内で稼働するLock（施錠）アプリのサンプルを公開しているようです。<br>
この中に、nRF5340 Lockアプリというサンプルがあるので、手始めにこのサンプルアプリについて調査します。

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
