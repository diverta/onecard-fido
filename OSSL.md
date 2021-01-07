# オープンソースコードライセンスについて

<b>[FIDO2アプリケーション](nRF5_SDK_v15.3.0)</b>、<b>[FIDO認証器管理ツール](MaintenanceTool)</b>内で使用されているオープンソースコード（フリーのライブラリー）についての概要を掲載します。

## オープンソースコード使用一覧

#### [FIDO2アプリケーション](nRF5_SDK_v15.3.0)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|1-1|nRF5 SDK|Nordic社の個別ライセンス|Nordic社製ICのみ搭載可|
|1-2|mbed TLS|Apache License Version 2.0|[注1]|
|1-3|[tinycbor](https://github.com/intel/tinycbor)|[MIT License](https://github.com/intel/tinycbor/blob/master/LICENSE)|[注2]|

[注1]完全オープンソースである、GPL v2.0バージョンも提供されていますが、このプロジェクトでは使用していません。nRF5 SDKのセットアップ媒体に同梱されていたものを、無修正で使用しています。<br>
[注2]あらかじめバイナリー形式ライブラリー（`libtinycbor.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](Research/FIDO_2_0/TINYCBOR.md)ご参照）。<br>

#### [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|2-1|[OpenSSL](https://github.com/openssl/openssl)|[Apache License Version 2.0](https://github.com/openssl/openssl/blob/master/LICENSE.txt)|[注1]|
|2-2|[tinycbor](https://github.com/intel/tinycbor)|[MIT License](https://github.com/intel/tinycbor/blob/master/LICENSE)|[注2]|

[注1]あらかじめバイナリー形式ライブラリー（`libcrypto.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](Research/Development/OPENSSLC.md)ご参照）。<br>
[注2]あらかじめバイナリー形式ライブラリー（`libtinycbor.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](Research/FIDO_2_0/TINYCBOR.md)ご参照）。<br>

#### [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|3-1|[peteroupc/CBOR](https://github.com/peteroupc/CBOR)|[CC0 1.0 Universal Public Domain Dedication](https://github.com/peteroupc/CBOR/blob/master/LICENSE.md)|[注1]|

[注1]2020/07/13現在、リポジトリーのソースコードをコピーしてVisual Studioプロジェクトに取り込み、無修正で使用しています。
