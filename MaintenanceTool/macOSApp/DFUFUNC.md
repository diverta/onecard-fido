# ファームウェア更新機能

## 概要
[USBブートローダー](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader)を導入した[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に対し、管理ツールから、ファームウェアを更新できる機能です。<br>
nRF52 SDKのDFU機能（[Secure DFU Bootloader over Serial Link (UART/USB)](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.3.0/sdk_app_serial_dfu_bootloader.html)）を使用して実装されています。

## 前提

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に、[USBブートローダー](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader)が導入されていることが前提となります。<br>
USBブートローダーの導入手順は、下記手順書をご参照願います。
- <b>[USBブートローダー書込手順](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/SB_JLINKSWDPROG.md)</b>

#### USBブートローダーのイメージ

USBブートローダーのイメージ（[mdbt50q_dongle.hex](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/mdbt50q_dongle.hex)）は、すでに下記手順により作成済です。

- <b>[USBブートローダー作成手順](../../nRF5_SDK_v15.3.0/examples/dfu/secure_bootloader/README.md)</b>


## 操作方法
2020/01/16現在、指示画面が未実装（後日対応予定）

## 開発情報

プログラム実装に関する情報を掲載いたします。

#### 指示画面(未実装)におけるコーディングについて

管理ツールの<b>ファームウェア更新（以下「DFU」）</b>機能は`ToolDFUCommand`というクラスで実装されています。<br>
指示画面クラスに、以下のようなプロパティーを定義し、インスタンス化して使用します。

```
@property (nonatomic) ToolDFUCommand    *toolDFUCommand;
:
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  :
  // DFU機能の初期設定
  [self setToolDFUCommand:[[ToolDFUCommand alloc] init]];
  :
}
```

メソッド`startDFUProcess`を呼び出すと、DFU処理実行が開始されます。<br>
下記は、IBAction（ボタン押下またはメニュー選択）により、DFU処理を実行させる例になります。

```
- (IBAction)menuItemDFUTestDidSelect:(id)sender {
    [[self toolDFUCommand] startDFUProcess];
}
```

DFU処理では、管理ツールに同梱されたファームウェア更新イメージファイル（`app_dfu_package.<バージョン文字列>.zip`）を、nRF52840に転送したのち、USB HID経由で、バージョン情報照会を行うよう実装しています。<br>
そのため、管理ツールで、USB HID接続が再開されたことを検知する必要があります。

以下のように、USB HID接続が検知されたことを`ToolDFUCommand`クラスに通知するよう実装します。

```
- (void)hidCommandDidDetectConnect {
    [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
    // DFU処理にHID接続開始を通知
    [[self toolDFUCommand] hidCommandDidDetectConnect:[self toolHIDCommand]];
}
```

#### DFUの流れについて

DFU機能は、以下の流れで行われます。

| # |項目 |処理内容 |
|:-:|:-|:-|
|1|処理タイムアウト監視開始|３０秒経過でタイムアウトと判定します。<br>（DFUの所要時間は21秒前後です）|
|2|DFU処理開始指示|サブスレッド上で行われます。|
|3|DFUイメージ抽出|ファームウェア更新イメージファイルを読込み、DFUイメージを抽出します。<br>また、イメージファイル名から更新バージョン文字列を取得します。<br>ファイル名は`app_dfu_package.<バージョン文字列>.zip`になります。|
|4|DFU対象デバイスに接続|nRF52840に、仮想COMポート経由で接続します。|
|5|DFU処理実行|nRF52840に、前述のDFUイメージを転送します。|
|6|DFU対象デバイスから切断|　|
|7|反映待機|DFUイメージ転送完了後、nRF52840アプリケーションが自動起動し、<br>USB HID接続が検出されるまで待機します。|
|8|バージョン情報照会|USB HID接続が再開されたら、HID経由でバージョン情報照会を実行します。|
|9|バージョンチェック|バージョン情報照会で取得したバージョン文字列をチェックし、<br>イメージファイル名から取得した更新バージョン文字列と等しければ、<br>DFU処理は完了となります。|


前述の `5`「DFU実行」は、以下の流れで行われます。

| # |項目 |処理内容 |
|:-:|:-|:-|
|1|DFU対象デバイスの通知設定|nRF52840とのデータ受け渡し（セッション）を開始します。|
|2|DFU対象デバイスからMTUを取得|DFUイメージのバイナリーデータ送信１回あたりのフレームサイズ<br>（MTU）を取得します。|
|3|datイメージ転送|プログラムイメージのFlash ROM配置の処理前に必要となる情報<br>（`Init packet`）を転送します。|
|4|binイメージ転送|Flash ROMに配置されるプログラムイメージを転送します。|

#### DFUセッションの内容

DFUの処理において受け渡しされるデータは以下のようになります。

| # |項目 |リクエスト | レスポンス |
|:-:|:-|:-|:-|
|0|DFU対象デバイスのPING [注1]|`09 ac c0`|`60 09 01 ac c0`|
|1|DFU対象デバイスの通知設定|`02 00 00 c0`|`60 02 01 c0`|
|2|DFU対象デバイスからMTUを取得|`07 c0`|`60 07 01 03 08 c0`<br>[注2]|
|3|datイメージ転送 [注3]|||
|3-1| SELECT OBJECT|`06 01 c0`|`60 06 01 00 02 00 00 00`<br>`00 00 00 00 00 00 00 c0`|
|3-2| CREATE OBJECT|`01 01 8e 00 00 00 c0`|`60 01 01 c0`|
|3-3| WRITE OBJECT|`08 <datイメージ> c0`<br>[注4]|なし|
|3-4| CRC GET|`03 c0`|`60 03 01 8e 00 00 00 a1`<br>`09 db dd fb c0`|
|3-5| EXECUTE OBJECT|`04 c0`|`60 04 01 c0`|
|4|binイメージ転送 [注5]|||
|4-1| SELECT OBJECT|`06 02 c0`|`60 06 01 00 10 00 00 00`<br>`00 00 00 00 00 00 00 c0`<br>[注6]|
|4-2| CREATE OBJECT|`01 02 00 10 00 00 c0`|`60 01 01 c0`|
|4-3| WRITE OBJECT|`08 <binイメージ> c0`<br>[注4]|なし|
|4-4| CRC GET|`03 c0`|`60 03 01 00 10 00 00 70`<br>`2e 06 f9 c0`|
|4-5| EXECUTE OBJECT|`04 c0`|`60 04 01 c0`|

[注1] 前述「DFU対象デバイスに接続」（メソッド`getConnectedDevicePath`）内で実行されます。DFU対象デバイスが接続されているかどうかのチェックを行うために実行します。<br>
[注2] レスポンスの4〜5バイト目に、リトルエンディアン形式で、MTUの最大値が格納されています。上記例では`0x0803`（2,051バイト）になります。ただし、nRF52の仕様上、イメージデータは4の倍数のサイズごとに送信する必要があるため、このケースでは実際のMTUを、2,048としております。<br>
[注3] `3-1`〜`3-5`の処理が、順番に１度だけ行われます。<br>
[注4] `<datイメージ>`にはdatイメージのバイト配列が、`<binイメージ>`にはbinイメージのバイト配列が入ります。また、nRF52の仕様上、特定のバイトデータをエスケープする必要があります（`c0`->`0xdbdc`, `0xdb`->`0xdbdd` に変換）。<br>
[注5] `4-1`の処理が１度だけ行われ、あとは１回あたり最大転送サイズ（4,096バイト）ごとに、`4-2`〜`4-5`の処理が繰り返されます。<br>
[注6] 前述「１回あたり最大転送サイズ」が、レスポンスの4〜5バイト目に、リトルエンディアン形式で格納されています。上記例では`0x1000`（4,096バイト）になります。
