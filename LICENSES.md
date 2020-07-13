# オープンソースコードライセンスについて

<b>[FIDO2アプリケーション](nRF5_SDK_v15.3.0)</b>、<b>[FIDO認証器管理ツール](MaintenanceTool)</b>内で使用されているオープンソースコード（フリーのライブラリー）についての概要を掲載します。

## オープンソースコード使用一覧

#### [FIDO2アプリケーション](nRF5_SDK_v15.3.0)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|1-1|nRF5 SDK|Nordic社の個別ライセンス|Nordic社製ICのみ搭載可|
|1-2|mbed TLS|Apache License Version 2.0|[注1]|
|1-3|[tinycbor](https://github.com/intel/tinycbor)|[MIT License](https://github.com/intel/tinycbor/blob/master/LICENSE)|[注2]|
|1-4|[cryptoauthlib](https://github.com/MicrochipTech/cryptoauthlib)|[Microchip社の個別ライセンス](https://github.com/MicrochipTech/cryptoauthlib/blob/master/license.txt)|Microchip社製ICのみ搭載可<br>[注3]|

[注1]完全オープンソースである、GPL v2.0バージョンも提供されていますが、このプロジェクトでは使用していません。nRF5 SDKのセットアップ媒体に同梱されていたものを、無修正で使用しています。<br>
[注2]あらかじめバイナリー形式ライブラリー（`libtinycbor.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](Research/FIDO_2_0/TINYCBOR.md)ご参照）。<br>
[注3]2020/07/13現在、Microchip社提供のライブラリーを無修正で使用していますが、ライセンス条項（Microchip社製ICのみ搭載可）に抵触するため、回避策を検討中です。

#### [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp)

後報

#### [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe)

後報
