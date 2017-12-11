# ビルド手順

## プロジェクト構成
* `components/`<br>
依存ライブラリーを収容<br>（ソースのカスタマイズはありません）

* `examples/diverta/`<br>
One Cardアプリケーションと、それにマージされたFIDO対応のコードを収容

* `examples/diverta/ble_u2f_lib`<br>
BLE U2Fサービスのコードを収容

* `external/`<br>
依存パッケージを収容<br>（ソースのカスタマイズはありません）

## 開発環境
nRF52DKを使用しています。
* BLE - nRF52832
* MCU - PCA10040
* ソフトデバイス - S132
* IDE - μVision 5

下記手順書を参照して、初期セットアップを行います。<br>
[../assets/nRF5開発環境インストール_V3.pdf](nRF5開発環境インストール_V3)

## μVisionを使用したビルド

### コードのチェックアウト

c:¥nordic（ディレクトリーがない場合は作成）配下に移動し、GitHubからコードをチェックアウトします。<br>
エクスプローラで見ると、下図のような感じになります。

<img src="../assets/0004.png" width="600">

### プロジェクト設定の確認

nRF52DKとPCを、USBケーブルで接続したのち、one_card_peripheral_app_pca10040_s132.uvprojx というファイルをダブルクリックします。

<img src="../assets/0005.png" width="600">

μVisionが起動したら、プロジェクト設定を確認します。

<img src="../assets/0006.png" width="800">

プロジェクト設定のポップアップを確認します。<br>
JLINK / J-TRACE Cortexが選択されていることを確認し、Settingsボタンをクリックします。

<img src="../assets/0007.png" width="500">

表示されたポップアップの、SW DeviceにDevice Nameが表示されていることを確認します。

<img src="../assets/0008.png" width="500">

### ビルドとプログラム書込み(Download)

ビルドを実行します。

<img src="../assets/0009.png" width="800">

ビルドの結果「0 Error(s)」となっていることを確認し、プログラムの書込みを実行します。

<img src="../assets/0010.png" width="800">

エラーが発生しなければ、下図のようになります。

<img src="../assets/0011.png" width="800">

これでプログラムの書込みは完了です。
