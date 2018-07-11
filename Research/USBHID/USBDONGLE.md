# USBドングルに関する調査

チップメーカーからリリースされている、USBドングルの評価基板を、U2F Registerで必須要件の「HIDデバイス」として利用できるかどうかの調査です。

- nRF51-Dongle (Nordic社) - HID使用不可

- BT800 Series (CSR社) - HID使用可能（HID Proxyモードを持つ。ただしOEMのみサポート）

## nRF51-Dongle

Nordicからリリースされている「nRF51-Dongle」というUSB BLEデバイスが、USB HIDデバイスとして利用できるかどうかを調査します。

万が一これが利用可能であれば、One Card、nRF51-Dongleというハード構成だけで、Firefox／Chromeブラウザー標準サポートによるU2F／WebAuthnが実現可能となります。<br>
ブラウザー標準サポートであれば、もちろんエクステンションの別途開発／導入は不要となります。

ただし当然のことながら、秘密鍵・証明書の発行やインストールは、従来通り、U2F管理ツールにより行う必要があります。

### 入手について

販売サイトは下記の通りです。（約 5,700yen）<br>
https://www.mouser.jp/ProductDetail/Nordic-Semiconductor/nRF51-Dongle?qs=4PbAv7ewtYyF2mxeEcEllA==

データシートは下記で公開されています。<br>
https://www.mouser.jp/datasheet/2/297/9233572-1131661.pdf

注意点として、日本の技適は未取得であるため、本格開発時は技適取得についても検討が必要となります。

### mbedサポート

「ARM mbed enabled」とあります。
https://os.mbed.com/platforms/Nordic-nRF51-Dongle/

BLE APIが利用できるかどうかは、実機にて要調査となりますが、BLE APIが利用できるとなれば、先行調査案件「mbedでUSBHIDデバイスを試す」による調査リソースがそのまま利用できる可能性が高いです。

mbedのBLE APIサポートに関する文献は下記になります。<br>
https://docs.mbed.com/docs/mbed-os-api-reference/en/5.1/APIs/communication/ble/

### HIDは利用不可

「nRF51-Dongle」は、シリアル通信インターフェースと、外部記憶インターフェースを実装しているようです。<br>
したがって、nRF51-DongleをPCのUSBポートに挿すと、PC側のOSからは仮想COMポート＆外付けドライブとして認識されます。

他方、残念ながらHIDインターフェースを持たないようなので、HIDデバイスとしては使用できません。

参考までに、下記URLでも、本件と同じようなことを検討されている方がいたようです。<br>
https://devzone.nordicsemi.com/f/nordic-q-a/17652/how-to-add-hid-on-nrf51-dongle <br>
おそらく、nRF51-DongleのコントローラーファームウェアにHIDインターフェースを実装するという案だと思われますが、ネイティブな処理系（ARM Keil MDK等）を使用して、アプリケーションを含むファームウェア全体を１から実装する必要があり、現実的ではないかと思われます。

## BT800 Series

CSR社からリリースされている、BLEデバイス「BT800 Series」には、HID Proxyという実装があるようです。<br>
http://assets.lairdtech.com/home/brandworld/files/Product%20Brief%20-%20BT800%20Series.pdf

USBに挿すと、ペアリング後はHIDデバイスとして認識されるとのことですが、残念ながら、CSR社のOEMでないとサポート（＝実装）されないということです。

#### 参考文献
http://www.softnavi.com/bluetooth.html <br>
https://www.digikey.jp/product-detail/ja/laird-wireless-thermal-systems/BT820/BT820-ND/4423863
