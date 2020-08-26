# セキュアIC組込み対応

## 概要
Microchip社のセキュアIC「[ATECC608A](https://www.mouser.jp/ProductDetail/Microchip-Technology/ATECC608A-MAHDA-T?qs=sGAEpiMZZMve4%2FbfQkoj%252BNx3hPbDs5d66otQ2I4K6nk%3D)」を、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)に追加導入します。

この対応により、nRF52840内部のFlash ROMにより管理されていた秘密鍵等の機密データが、（ハードウェア仕様的に秘密鍵等を外部漏洩不可としている）ATECC608Aにより管理されることとなり、結果としてセキュリティー性が向上することになるかと存じます。

## ATECC608Aの特徴

- ATECC608Aは、3mm x 2mm の超小型セキュアICです。

- nRF52840との通信は、I2Cインターフェース（TWI）によるシリアル通信で行われます。

- FIDO機能で使用するEC鍵ペアやECDSA署名、ECDH共通鍵、SHA256ハッシュ等の生成機能を有しています。

- 任意の外部秘密鍵や外部データを格納することができます。<br>
ただし外部秘密鍵は、ひとたびATECC608A内に格納されると、いかなる方法によっても、その内容を参照することはできなくなります。

- 任意の外部暗号を使用したデータ暗号化／復号化ができます。<br>
暗号化アルゴリズムは「AES-128」になります。<br>
ただし外部暗号は、ひとたびATECC608A内に格納されると、いかなる方法によっても、その内容を参照することはできなくなります。

## 対応プラン

- まずFIDO機能で使用する秘密鍵／暗号化関連処理を、ATECC608Aを使用した処理で置き換えます。<br>
FIDO機能では、[管理ツール](../MaintenanceTool/README.md)を使用し、ATECC608A外部から秘密鍵をインポートする運用としております。

- 次に、PIVやOpenPGP等の[スマートカード・エミュレーション機能](../CCID/README.md)においても、ATECC608Aを使用して鍵管理を行うものとします。<br>
近日本格開発予定のPIV機能では、[Yubico PIV Toolによる初期データ導入](../Research/CCID/SETUPYKPIV.md)を想定しているため、ATECC608A外部から秘密鍵をインポートするのではなく、内部で秘密鍵を生成し、外部に公開鍵をエクスポートする実装にしたいと考えております。

## 開発情報

#### [ATECC608A専用ライブラリー](../SECUREIC/atecc_lib/README.md)

ATECC608AをnRF52840とI2C接続し、セキュアICとして利用するためのモジュールです。
