# オープンソースコードライセンスについて

最終更新日：2023/2/8

<b>[nRF52840アプリケーション](nRF52840_app)</b>、<b>[FIDO認証器管理ツール](MaintenanceTool)</b>内で使用されているオープンソースコード（フリーのライブラリー）についての概要を掲載します。

## オープンソースコード使用一覧

#### [nRF52840アプリケーション](nRF52840_app)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|1-1|nRF5 SDK|Nordic社の個別ライセンス|Nordic社製ICのみ搭載可|
|1-2|mbed TLS|Apache License Version 2.0|[注1]|
|1-3|[tinycbor](https://github.com/intel/tinycbor)|[MIT License](https://github.com/intel/tinycbor/blob/master/LICENSE)|[注2]|

[注1]完全オープンソースである、GPL v2.0バージョンも提供されていますが、このプロジェクトでは使用していません。nRF5 SDKのセットアップ媒体に同梱されていたものを、無修正で使用しています。<br>
[注2]あらかじめバイナリー形式ライブラリー（`libtinycbor.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](nRF52840_app/firmwares/secure_device_app/TINYCBOR.md)ご参照）。<br>

#### [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/README.md)

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|2-1|[OpenSSL](https://github.com/openssl/openssl)|[Apache License Version 2.0](https://github.com/openssl/openssl/blob/master/LICENSE.txt)|[注1]|
|2-2|[tinycbor](https://github.com/intel/tinycbor)|[MIT License](https://github.com/intel/tinycbor/blob/master/LICENSE)|[注2]|

[注1]あらかじめバイナリー形式ライブラリー（`libcrypto.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](Research/Development/OPENSSLC.md)ご参照）。<br>
[注2]あらかじめバイナリー形式ライブラリー（`libtinycbor.a`）を作成し、それをこのプロジェクトに組み込んで使用しています（詳細は[こちらの手順書](nRF52840_app/firmwares/secure_device_app/TINYCBOR.md)ご参照）。<br>

#### [Windows版 FIDO認証器管理ツール](MaintenanceTool/dotNET/README.md)

Microsoft社以外のフリーライブラリーについて掲載いたしました。

|#|ライブラリー名|ライセンス|備考|
|:---:|:---|:---|:---|
|3-1|[PeterO.Cbor](https://github.com/peteroupc/CBOR)|[Creative Commons Zero (CC0)](https://creativecommons.org/publicdomain/zero/1.0/)|[注1]|
|3-2|[QRCodeDecoder](https://github.com/StefH/QRCode)|[The Code Project Open License (CPOL) 1.02](https://www.codeproject.com/info/cpol10.aspx)|[注2]|

[注1]Visual Studioのプロジェクトに、NuGetパッケージとして導入しています。バンドルには`CBOR.dll`として格納されています。<br>
[注2]Visual Studioのプロジェクトに、NuGetパッケージとして導入しています。バンドルには`QRCodeDecoderLibrary.dll`としてで格納されています。
