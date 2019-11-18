# 管理ツールのログファイル

## 概要
FIDO認証器管理ツールから出力されるログファイルについて説明します。

## ログファイルの場所

FIDO認証器管理ツールの実行中に出力されるログは、macOSのユーザーディレクトリー配下のログファイル（下記の場所）に保存されます。

`$HOME/Library/Logs/Diverta/FIDO/MaintenanceTool.log`

適宜、テキストエディターにより参照することが可能です。

また、コマンド`tail -f $HOME/Library/Logs/Diverta/FIDO/MaintenanceTool.log`を実行することにより、実行中に出力されるログをリアルタイム参照することも可能です。<br>
（macOSのターミナルアプリによるログ参照と等価の動きになります）

## ログファイルの内容

macOSアプリの一般的な形式で出力されます。<br>
ログの出力イメージは以下のようになります。
```
2019-11-13 15:50:07.036 [info] FIDO認証器管理ツールを起動しました: Version 0.1.21
2019-11-13 15:50:07.038 [info] USBデバイス検知を開始しました。
2019-11-13 15:50:07.044 [info] USB HIDデバイスに接続されました。
2019-11-13 15:50:14.305 [info] HID PINGテストを開始します。
2019-11-13 15:50:14.305 [debug] HID Sent INIT frame: data size=8 length=8
ff ff ff ff 86 00 08 71 cb 1c 3b 10 8e c9 24
2019-11-13 15:50:14.319 [debug] HID Recv INIT frame: data size=17 length=17
ff ff ff ff 86 00 11 71 cb 1c 3b 10 8e c9 24 01
00 33 01 02 05 00 02 07
2019-11-13 15:50:14.320 [debug] HID Sent INIT frame: data size=100 length=57
01 00 33 01 81 00 64 cd 56 82 4a 02 68 be ec 6d
db 63 4a f8 47 09 9a d8 29 e8 db 39 31 37 6b a8
2d 36 20 eb 13 6f 34 33 72 94 bc 56 82 e9 75 cb
72 03 81 ac 2d 92 4f ec c3 9d cb f2 e8 16 ba ae
2019-11-13 15:50:14.320 [debug] HID Sent CONT frame: seq=0 length=43
01 00 33 01 00 39 71 b0 5b 23 c0 b0 eb 09 8b c1
17 f0 d4 22 68 ff f3 a0 9c 92 79 34 67 25 bf 4d
62 6d a9 b7 73 eb c6 b8 8c 8c 6b 36 98 9f 5e d9
2019-11-13 15:50:14.339 [debug] HID Recv INIT frame: data size=100 length=57
01 00 33 01 81 00 64 cd 56 82 4a 02 68 be ec 6d
db 63 4a f8 47 09 9a d8 29 e8 db 39 31 37 6b a8
2d 36 20 eb 13 6f 34 33 72 94 bc 56 82 e9 75 cb
72 03 81 ac 2d 92 4f ec c3 9d cb f2 e8 16 ba ae
2019-11-13 15:50:14.347 [debug] HID Recv CONT frame: seq=0 length=43
01 00 33 01 00 39 71 b0 5b 23 c0 b0 eb 09 8b c1
17 f0 d4 22 68 ff f3 a0 9c 92 79 34 67 25 bf 4d
62 6d a9 b7 73 eb c6 b8 8c 8c 6b 36 98 9f 5e d9
2019-11-13 15:50:14.348 [info] HID PINGテストが成功しました。
2019-11-13 15:50:18.381 [info] USB HIDデバイスが取り外されました。
2019-11-13 15:50:26.233 [info] BLE PINGテストを開始します。
2019-11-13 15:50:26.234 [debug] BLE Sent INIT frame: data size=100 length=61
81 00 64 e0 46 e5 2a e4 e6 4b fe 6f 95 b1 94 9e
cf 8f 37 17 c6 65 d9 1a d9 e4 28 14 f3 b6 e8 84
b5 44 a0 4b 90 de d7 3d ee 94 65 b0 fe 09 6e 7b
06 0e e0 b6 10 4b 15 0a 73 c6 7f a9 06 9e 76 46
2019-11-13 15:50:26.234 [debug] BLE Sent CONT frame: seq=0 length=39
00 ac c5 2c ed 84 9a fe 1c cc 43 4a 24 d9 be 1e
fe fe 51 11 8d 94 27 dd e6 4f 84 41 db c8 00 32
d7 9e 31 1c c6 d8 a5 0f
2019-11-13 15:50:26.235 [info] FIDO認証器のスキャンを開始します。
2019-11-13 15:50:27.279 [info] FIDO認証器のスキャンを停止しました。
2019-11-13 15:50:29.630 [info] FIDO認証器に接続しました。
2019-11-13 15:50:29.633 [info] FIDO BLEサービスが見つかりました。
2019-11-13 15:50:31.331 [info] 受信データの監視を開始します。
2019-11-13 15:50:32.231 [info] リクエストを送信しました。
2019-11-13 15:50:32.233 [debug] BLE Recv INIT frame: data size=100 length=61
81 00 64 e0 46 e5 2a e4 e6 4b fe 6f 95 b1 94 9e
cf 8f 37 17 c6 65 d9 1a d9 e4 28 14 f3 b6 e8 84
b5 44 a0 4b 90 de d7 3d ee 94 65 b0 fe 09 6e 7b
06 0e e0 b6 10 4b 15 0a 73 c6 7f a9 06 9e 76 46
2019-11-13 15:50:32.382 [debug] BLE Recv CONT frame: seq=0 length=39
00 ac c5 2c ed 84 9a fe 1c cc 43 4a 24 d9 be 1e
fe fe 51 11 8d 94 27 dd e6 4f 84 41 db c8 00 32
d7 9e 31 1c c6 d8 a5 0f
2019-11-13 15:50:32.384 [info] レスポンスを受信しました。
2019-11-13 15:50:32.554 [info] FIDO認証器の接続が切断されました。
2019-11-13 15:50:32.556 [info] BLE PINGテストが成功しました。
2019-11-13 15:50:36.654 [info] FIDO認証器管理ツールを終了しました。
```
