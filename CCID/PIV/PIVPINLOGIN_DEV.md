# PIN番号を使用したmacOSログイン時の動作

## 概要
[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に設定した[PIV機能](../../CCID/ccid_lib/README.md)を使用し、PIN番号によるmacOSログイン時、PC〜nRF52840間で行われるやり取りについて掲載しています。

## スマートカードペアリング実行時の動作

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook、iMac等）のUSBポートに装着すると、CCID I/Fを経由し、PIV設定情報が取得されます。<br>
この時のPC〜nRF52840間のやり取りの流れは以下になります。
- nRF52840のPIVアプレットをSELECT
- nRF52840からCHUIDを取得
- nRF52840からPIV認証用／電子署名用／管理機能用の証明書を取得

<b>【nRF52840側のデバッグプリント】</b>
```
<debug> usbd_service: usbd_init() done
：
<info> application_init: Diverta FIDO Authenticator application started.
<debug> app: Slot power on
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: Card Holder Unique Identifier is requested (61 bytes)
<debug> app: APDU send: SW(9000) data(61)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: APDU send: SW(6A82)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for PIV Authentication is requested (921 bytes)
<debug> app: APDU send: SW(61FF) data(921)
<debug> app: APDU send: SW(61FF) data(255, total 921)
<debug> app: APDU send: SW(619B) data(255, total 921)
<debug> app: APDU send: SW(9000) data(155, total 921)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: APDU send: SW(6A82)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for Digital Signature is requested (410 bytes)
<debug> app: APDU send: SW(619A) data(410)
<debug> app: APDU send: SW(9000) data(154, total 410)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for Key Management is requested (411 bytes)
<debug> app: APDU send: SW(619B) data(411)
<debug> app: APDU send: SW(9000) data(155, total 411)
：
<debug> app: Slot power off
```

その後、スマートカードペアリングを実行時は、下図のようなダイアログが表示されます。

<img src="assets03/0003.jpg" width="400">

このダイアログでパスワードを入力するか、Touch IDを使用すると、その後ほどなく、nRF52840側のPIVアプレットがSELECTされます。

<b>【nRF52840側のデバッグプリント】</b>
```
<debug> app: Slot power on
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
```

レスポンスが戻ると、下図のようなPIN番号入力画面が表示されます。<br>
PIN番号を入力し「OK」をクリックします。

<img src="assets03/0004.jpg" width="400">

入力されたPIN番号により、nRF52840側で`Internal authenticate`が実行されます。<br>
流れとしては以下になります。
- PCからPIN認証リクエストが送信 --> nRF52840からOKステータス（0x9000）が戻る
- PCからRSA-2048証明書を使用した署名が送信 --> nRF52840側からRSA-2048秘密鍵を使用した署名が戻る<br>
（PC側では、PIV認証用の証明書に添付の公開鍵が使用され、nRF52840側では、PIV認証用の秘密鍵が使用されています）
- PC側で署名の検証が行われ、OKだとキーチェーン登録処理へ進むことができる

<b>【nRF52840側のデバッグプリント】</b>
```
<debug> app: APDU recv: CLA INS P1 P2(00 20 00 80) Lc(8) Le(256)
<info> app: PIV PIN verification success
<debug> app: APDU send: SW(9000)
<debug> app: APDU send: SW(9000)
<debug> app: APDU recv: CLA INS P1 P2(00 87 07 9A) Lc(266) Le(256)
<debug> app: Tag 0x82, pos: 6, len: 0
<debug> app: Tag 0x81, pos: 10, len: 256
<debug> app: internal authenticate
<debug> app: Private Key for PIV application is requested: tag=0x9A (640 bytes)
<debug> app: internal authenticate (RSA2048) done
<debug> app: APDU send: SW(6108) data(264)
<debug> app: APDU send: SW(9000) data(8, total 264)
```

下図のようなダイアログが表示され、キーチェーンのパスワードを指定すると、キーチェーン登録処理が行われ、スマートカードペアリングが完了します。

<img src="assets03/0005.jpg" width="400">

## macOSにログイン時の動作

macOSのログイン画面が表示された後、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook、iMac等）のUSBポートに装着すると、下図のように、ユーザーパスワードではなく、PIN番号を入力するためのボックスが表示されます。

<img src="assets03/0007.jpg" width="400">

ここまでのPC〜nRF52840間のやり取りの流れは以下になります。

- nRF52840のPIVアプレットをSELECT
- nRF52840からCHUIDを取得
- nRF52840からPIV認証用／電子署名用／管理機能用の証明書を取得

<b>【nRF52840側のデバッグプリント】</b>
```
<debug> app: Slot power on
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: Card Holder Unique Identifier is requested (61 bytes)
<debug> app: APDU send: SW(9000) data(61)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: APDU send: SW(6A82)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for PIV Authentication is requested (921 bytes)
<debug> app: APDU send: SW(61FF) data(921)
<debug> app: APDU send: SW(61FF) data(255, total 921)
<debug> app: APDU send: SW(619B) data(255, total 921)
<debug> app: APDU send: SW(9000) data(155, total 921)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: APDU send: SW(6A82)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for Digital Signature is requested (410 bytes)
<debug> app: APDU send: SW(619A) data(410)
<debug> app: APDU send: SW(9000) data(154, total 410)
<debug> app: APDU recv: CLA INS P1 P2(00 CB 3F FF) Lc(5) Le(256)
<debug> app: X.509 Certificate for Key Management is requested (411 bytes)
<debug> app: APDU send: SW(619B) data(411)
<debug> app: APDU send: SW(9000) data(155, total 411)
：
<debug> app: Slot power off
```

PIN番号を入力して、ログインを実行します。

<img src="assets03/0008.jpg" width="400">

ログイン実行時の、PC〜nRF52840間のやり取りの流れは以下になります。

- nRF52840のPIVアプレットをSELECT
- 入力されたPIN番号により、nRF52840側で`Internal authenticate`実行<br>（前述の通り）
- PCから送信された公開鍵と、nRF52840側に導入されている秘密鍵により、ECDH共通鍵を生成し、PCへ転送<br>
（PC側では、管理機能用の証明書に添付の公開鍵が使用され、nRF52840側では、管理機能用の秘密鍵が使用されています）

<b>【nRF52840側のデバッグプリント】</b>
```
<debug> app: Slot power on
<debug> app: APDU recv: CLA INS P1 P2(00 A4 04 00) Lc(11) Le(256)
<debug> app: select_applet: applet switched to PIV
<debug> app: APDU send: SW(9000) data(19)
<debug> app: APDU recv: CLA INS P1 P2(00 20 00 80) Lc(8) Le(256)
<info> app: PIV PIN verification success
<debug> app: APDU send: SW(9000)
<debug> app: APDU send: SW(9000)
<debug> app: APDU recv: CLA INS P1 P2(00 87 07 9A) Lc(266) Le(256)
<debug> app: Tag 0x82, pos: 6, len: 0
<debug> app: Tag 0x81, pos: 10, len: 256
<debug> app: internal authenticate
<debug> app: Private Key for PIV application is requested: tag=0x9A (640 bytes)
<debug> app: internal authenticate (RSA2048) done
<debug> app: APDU send: SW(6108) data(264)
<debug> app: APDU send: SW(9000) data(8, total 264)
<debug> app: APDU recv: CLA INS P1 P2(00 20 00 80) Lc(8) Le(256)
<info> app: PIV PIN verification success
<debug> app: APDU send: SW(9000)
<debug> app: APDU recv: CLA INS P1 P2(00 87 11 9D) Lc(71) Le(256)
<debug> app: Tag 0x82, pos: 4, len: 0
<debug> app: Tag 0x85, pos: 6, len: 65
<debug> app: ECDH with the PIV KMK
<debug> app: Private Key for PIV application is requested: tag=0x9D (32 bytes)
<debug> fido_crypto: Compute shared secret using ECDH start
<debug> fido_crypto: Compute shared secret using ECDH end
<debug> app: APDU send: SW(9000) data(36)
：
<debug> app: Slot power off
```
