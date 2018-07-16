# USB HIDデバイス関連調査

## 概要

Chromeブラウザーを使用して、GoogleアカウントのU2F Registerを実行するために最小限必要となる、USB HIDデバイスの開発について調査を行います。

2018/7/9現在、USB HIDデバイスとU2F管理ツール、One Cardを利用して、GoogleアカウントのU2F Register／Authenticateが実行できたことが確認できております。

（確認時は、カスタマイズされたエクステンション等は一切使用せず、Chromeブラウザー標準サポート機能だけを使用しております）

## 確認手順／結果

- <b>[AndroidスマートフォンからU2F Authenticateを実行](NUCLEO_HID_U2F_AUTHENTICATE.md)</b><br>
One Cardを使用し、PCでU2F RegisterしたGoogleアカウントで、AndroidスマートフォンからU2F Authenticateを実行させる手順と結果になります。

## 調査内容

- <b>[HIDマウスデバイスを試す](NUCLEO_HID_MOUSE.md)</b><br>
まずは手始めに、NUCLEO（STM32開発環境）＋mbedサンプルアプリにより、HIDマウスデバイスを試しました。

- <b>[HID U2Fデバイス開発の準備](NUCLEO_HID_U2F_PREPARE.md)</b><br>
前項テストで使用したハードウェアを、HID U2Fデバイスとして認識させるところまで試します。

- <b>[HID U2Fデバイスにコマンドを実装](NUCLEO_HID_U2F_COMMAND.md)</b><br>
前述のHID U2Fデバイスに、U2F HID Init／U2F Versionを実行させるためのコマンドを実装するところまで試します。

- <b>[HID U2FデバイスとU2F管理ツールの連携](NUCLEO_HID_U2FMNT.md)</b><br>
前述のHID U2Fデバイスを、既存のU2F管理ツールと連携させるところまで試します。

- <b>[HID U2FデバイスからU2F Registerを実行](NUCLEO_HID_U2F_REGISTER.md)</b><br>
前述のHID U2Fデバイスから、U2F管理ツールを経て、One CardのU2F Registerコマンドを実行させるところまで試します。

## 参考調査

- <b>[NUCLEOのダウンサイジングに関する調査](NUCLEO_DOWNSIZE.md)</b><br>
上記動作確認時に使用した「NUCLEO-F411RE」を「NUCLEO-F103RB」にダウンサイズ可能かどうか調査しました。<br>
結果としては、mbedによるダウンサイジングは不可能と判断しました。

- <b>[USBドングルに関する調査](USBDONGLE.md)</b><br>
チップメーカーからリリースされている、USBドングルの評価基板を、U2F Registerで必須要件の「HIDデバイス」として利用できるかどうかの調査です。
