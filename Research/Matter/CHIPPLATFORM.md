# nRF Connect CHIPプラットフォームについて

nRF5340の標準プラットフォームである「nRF Connect SDK」をベースとし、Matterアプリケーションを実装するプラットフォームモデルです。

#### CHIPプラットフォーム

nRF Connect CHIPプラットフォームは、Nordic SemiconductorのnRF Connect SDKを使用するプラットフォームです。<br>
下図は、nRF Connectプラットフォームで実行され、通信にBLE／Threadスタックを使用するCHIPアプリケーションの、簡略化された構造を示しています。

<img src="https://raw.githubusercontent.com/project-chip/connectedhomeip/master/docs/images/chip_nrfconnect_overview_simplified.svg" width=640>

注：読みやすくするために、図にはすべてのプロジェクトコンポーネントが示されているわけではなく、一般的なCHIPアプリケーションにとって最も重要なコンポーネントのみが示されています。

#### nRF Connect SDK

Nordic SemiconductorのnRF Connect SDKを使用すると、セルラーIoT（`LTE-M`、`NB-IoT`）、Bluetooth Low Energy、Thread、Zigbee、Bluetoothメッシュなどのさまざまなアプリケーションを構築できます。

SDKには、Nordic SemiconductorのnRF9160、nRF5340、およびnRF52シリーズデバイス用のサンプル、ライブラリ、およびドライバーのフルセットが含まれています。

nRF Connect SDKは、接続されたリソースに制約のあるデバイス専用のスケーラブルなリアルタイムオペレーティングシステムであるZephyr RTOSをベースとしています。<br>
Zephyrは、複数のハードウェアプラットフォームをサポートし、ハードウェアドライバー、アプリケーションプロトコル、プロトコルスタックなどを提供します。

Zephyrに加えて、nRF Connect SDKは、暗号ライブラリmbedTLS、MCUブートローダー、またはThreadスタックの`OpenThread`実装などの他のプロジェクトも統合します。

（訳者注：nRF Connect SDKアプリケーション基盤は現在、鋭意開発中です。「[nRF5340アプリケーション開発に関する調査](https://github.com/diverta/onecard-fido/projects/2)」をご参照）

#### BLE／Threadスタック

nRF Connectプラットフォームアプリケーションでは、BLEインターフェイスを使用して、CHIPデバイスとCHIPコントローラー間のペアリング、およびThreadネットワークプロビジョニング操作を実行します。<br>
その後、完全にプロビジョニングされたデバイスは、Threadネットワーク内の他のデバイスと通信できるようになります。

BLE通信のために、nRF Connectプラットフォームアプリケーションは、BLEスタックを使用しています。<br>
BLEホスト部分はZephyr RTOSによって提供され、SoftDeviceコントローラーはnRF Connect SDKのドライバーに実装されています。

Thread通信のために、nRF Connectプラットフォームアプリケーションは、さまざまなプロジェクトに実装された複数のレイヤーで構成されるThreadスタックを使用しています。<br>
Threadスタックのコアは`OpenThread`ですが、nRF Connect SDKによって提供されるIEEE 802.15.4無線ドライバーと、Zephyrによって提供されるネットワーク層機能も必要です。

nRF Connect SDKのマルチプロトコルサービスレイヤー（MPSL）ドライバーを使用すると、同じ無線チップ上でBLEとThreadを同時に実行できます。

#### CHIPインテグレーション

CHIPは、ネットワークの観点から見て、提示されたモデルの最上位のアプリケーション層にあります。<br>
nRF Connect SDKおよびZephyrによって提供されるBLE／Threadスタックは、所定の中間レイヤーを使用し、CHIPスタックと統合する必要があります。

実際には、このレイヤーには、CHIPスタックで定義された抽象マネージャーインターフェイス（BLEマネージャーやThreadスタックマネージャーなど）のプラットフォーム固有の実装が含まれています。 <br>
アプリケーションはCHIPのプラットフォームに依存しないインターフェースを使用でき、CHIPスタックを介して通信を実行するために追加のプラットフォーム関連のアクションは必要ありません。

#### ビルドシステム

nRF Connectプラットフォームは、下記ビルドシステムを使用して`Ninja`ビルドスクリプトを生成します。

- GN - CHIPプロジェクト内で使用されるケースがほとんどになります。
- CMake - nRF Connect CHIPプラットフォームに関連する他のコンポーネント、すなわち、nRF Connect SDKとZephyr内で使用されます。

結果として、CHIPのスタック／プラットフォームモジュールはGNで構築され（概要図を参照）、出力はライブラリファイルの生成に使用されます。<br>
アプリケーション、nRF Connect SDK、およびZephyrはCMakeを使用して構築され、CHIPライブラリファイルはコンパイル処理工程内でインポート（リンク）されます。
