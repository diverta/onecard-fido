# U2F USB HIDデバイス用ファームウェア

## プログラムについて

本ファームウェアは、mbedのサンプル・プログラム `Nucleo_usbmouse` に修正／追加を施して制作しております。<br>
https://os.mbed.com/teams/ST/code/Nucleo_usbmouse/

オリジナルから変更した `main.cpp` と、U2F HIDデバイスに特化したライブラリー `USBU2FAuthenticator` から構成されます。

## ライブラリー `USBDEVICE` について

本ファームウェアはmbedのライブラリー `USBDEVICE` を使用しています。<br>
https://os.mbed.com/teams/ST/code/USBDEVICE/

ただし、２点のインターフェースを実装する必要があったため、オリジナル・コードに直接手を入れて修正しています。

変更箇所は下記の２点になります。

#### USBDEVICE/USBDevice/USBHAL_STM32F4.cpp

```
136,137c136,142
<             type = 2;
<             break;
---
>             //
>             // mbed original から変更:
>             //  EP2を使用するインターフェースの
>             //  type を 3 に設定
>             //
>             // type = 2;
>             // break;
```

#### USBDEVICE/USBHID/USBHID.cpp

```
108a109,120
>
>                             //
>                             // mbed original から変更:
>                             //  EP2を使用するインターフェースの
>                             //  USAGE_PAGE／USAGEを
>                             //  0xff00 に設定
>                             //
>                             if (transfer->setup.wIndex == 1) {
>                                 transfer->ptr[1] = 0x00;
>                                 transfer->ptr[2] = 0xff;
>                             }
>
173a186,188
>     addEndpoint(EPBULK_IN, MAX_PACKET_SIZE_EPINT);  // EP2_IN
>     addEndpoint(EPBULK_OUT, MAX_PACKET_SIZE_EPINT); // EP2_OUT
>
```
