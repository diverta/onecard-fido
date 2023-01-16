# 管理ツールのログファイル

## 概要
[FIDO認証器管理ツール（ベンダー向け）](../../MaintenanceTool/macOSApp/DEVTOOL.md)から出力されるログファイルについて説明します。

## ログファイルの場所

FIDO認証器管理ツールの実行中に出力されるログは、macOSのユーザーディレクトリー配下のログファイル（下記の場所）に保存されます。

`$HOME/Library/Logs/Diverta/FIDO/VendorMaintenanceTool.log`

具体的には、例えば `/Users/user/Library/Logs/Diverta/FIDO/VendorMaintenanceTool.log` といったパスになります。

### ログファイルの場所を開く

ログファイルの格納場所を、macOSのFinderで開くことができます。<br>
管理ツールの「ユーティリティー」画面で「管理ツールのログを参照」ボタンをクリックします。

<img src="assets08/0002.jpg" width="400">

ログファイル「`VendorMaintenanceTool.log`」を格納するディレクトリーが、Finderで表示されます。

<img src="assets08/0003.jpg" width="410">

ログファイルは通常のテキストファイルですので、適宜、テキストエディターにより内容を参照することが可能です。

また、コマンド`tail -f $HOME/Library/Logs/Diverta/FIDO/VendorMaintenanceTool.log`を実行することにより、実行中に出力されるログをリアルタイム参照することも可能です。<br>
（macOSのターミナルアプリによるログ参照と等価の動きになります）

## ログファイルの内容

macOSアプリの一般的な形式で出力されます。<br>
ログの出力イメージは以下のようになります。
```
2023-01-13 16:51:19.149 [info] FIDO認証器管理ツールを起動しました: Version 0.2.1
2023-01-13 16:59:25.117 [info] USB HIDデバイスに接続されました。
2023-01-13 17:00:54.224 [info] 鍵・証明書インストールを開始します。
2023-01-13 17:00:54.224 [debug] HID Sent INIT frame: data size=8 length=8
ff ff ff ff 86 00 08 71 cb 1c 3b 10 8e c9 24
2023-01-13 17:00:54.246 [debug] HID Recv INIT frame: data size=17 length=17
ff ff ff ff 86 00 11 71 cb 1c 3b 10 8e c9 24 01
00 33 01 02 05 00 02 07
2023-01-13 17:00:54.248 [debug] HID Sent INIT frame: data size=613 length=57
01 00 33 01 c0 02 65 48 cf 91 6d 82 30 03 cb 4b
4d c8 6a ff 05 14 49 c1 f4 11 fc 67 37 b7 3a 71
53 a3 4e 65 0d 03 95 d0 30 82 02 40 30 82 01 e6
a0 03 02 01 02 02 01 00 30 09 06 07 2a 86 48 ce
2023-01-13 17:00:54.248 [debug] HID Sent CONT frame: seq=0 length=59
01 00 33 01 00 3d 04 01 30 7e 31 0b 30 09 06 03
55 04 06 13 02 4a 50 31 0e 30 0c 06 03 55 04 08
13 05 54 6f 6b 79 6f 31 14 30 12 06 03 55 04 07
13 0b 53 68 69 6e 6a 75 6b 75 2d 6b 75 31 15 30
:
2023-01-13 17:00:54.251 [debug] HID Sent CONT frame: seq=9 length=25
01 00 33 01 09 5b a9 26 13 94 5c 91 39 ac 6e 71
4c 29 73 4b 68 d2 06 55 ef b6 4a ab 49 af
2023-01-13 17:00:54.350 [debug] HID Recv INIT frame: data size=1 length=1
01 00 33 01 c0 00 01 00
2023-01-13 17:00:54.352 [info] 鍵・証明書インストールが成功しました。
2023-01-13 17:00:58.376 [info] FIDO認証器管理ツールを終了しました。
```
