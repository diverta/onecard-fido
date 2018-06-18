# USB HIDデバイス初期調査

## 概要

Chromeブラウザーを使用して、GoogleアカウントのU2F Registerを実行するために最小限必要となる、USB HIDデバイスの開発について、初期段階調査を行います。

## 調査内容

- <b>[HIDマウスデバイスを試す](NUCLEO_HID_MOUSE.md)</b><br>
まずは手始めに、NUCLEO（STM32開発環境）＋mbedサンプルアプリにより、HIDマウスデバイスを試しました。

- <b>[HID U2Fデバイス開発の準備](NUCLEO_HID_U2F_PREPARE.md)</b><br>
[WIP] 前項テストで使用したハードウェアを、HID U2Fデバイスとして認識させるところまで試します。

- <b>[HID U2Fデバイスにコマンドを実装](NUCLEO_HID_U2F_COMMAND.md)</b><br>
[WIP] 前項HID U2Fデバイスに、U2F Versionを実行させるためのコマンドを実装するところまで試します。

- <b>[HID U2FデバイスとU2F管理ツールの連携](NUCLEO_HID_U2F_MNTTOOL.md)</b><br>
[WIP] 前項HID U2Fデバイスを、既存のU2F管理ツールと連携させるところまで試します。

- <b>[HID U2FデバイスからU2F Versionを実行](NUCLEO_HID_U2F_VERSION.md)</b><br>
[WIP] 前項HID U2Fデバイスから、U2F管理ツールを経て、One CardのU2F Versionコマンドを実行させるところまで試します。

## 調査が完了したら

次のステップとして「HID U2FデバイスからU2F Registerを実行」できるような仕組みの開発に移行します。
