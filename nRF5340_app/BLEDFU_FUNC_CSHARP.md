# ファームウェア更新機能（BLE）

最終更新日：2022/3/23

## 概要
ブートローダー「[MCUboot](https://www.mcuboot.com/documentation/readme-zephyr/)」を導入したFIDO認証器に対し、管理ツールから、ファームウェアを更新できる機能です。<br>
nRF Connect SDKのBLE DFU機能（[GATT DFU SMP Service Client](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/bluetooth_services/services/dfu_smp.html)）を使用して実装されています。

## 前提

FIDO認証器に、<b>ブートローダーを組み込んだ[nRF5340アプリケーション](../nRF5340_app/secure_device_app)</b>が導入されていることが前提となります。

## プログラムの実装

プログラム実装に関する情報を掲載いたします。

#### 関連モジュール

Windows版管理ツールの関連モジュールは以下になります。

|# |モジュール名 |内容 |備考|
|:-:|:-|:-|:-|
|1|`BLEDFUProcessingForm`|処理進捗画面||
|2|`BLEDFUStartForm`|処理開始画面||
|3|`BLESMPCBORDecoder`|DFUレスポンス（CBOR形式）を解析するモジュール||
|4|`ToolBLEDFU`|DFUコマンドクラス||
|5|`ToolBLEDFUProcess`|DFUトランザクションクラス||
|6|`ToolBLEDFUImage`|ファームウェア更新イメージを扱うモジュール||
|7|`ToolBLESMPService`|BLE SMPサービス操作モジュール||

#### メニュー選択〜処理開始画面の表示

管理ツールの<b>ファームウェア更新（以下「DFU」）</b>機能は`ToolBLEDFU`というクラス（以下「DFU処理クラス」）で実装されています。<br>
メイン画面（`MainForm`）の起動のタイミングで、DFU処理クラスをインスタンス化します。

```
//
// MaintenanceToolGUI/MainForm.cs
//
namespace MaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private BLEMain ble;
        :
        private ToolBLEDFU toolBLEDFU;
        :
        public MainForm()
        {
            :
            // BLE処理クラスを生成
            // （バージョン情報照会で使用します）
            ble = new BLEMain(this);
            :
            // DFU処理クラスを生成
            toolBLEDFU = new ToolBLEDFU(this, ble);
            :
```

メイン画面のメニュー選択により、メソッド`DoCommandBLEDFU`を呼び出すと、DFU処理実行が開始されます。

```
//
// MaintenanceToolGUI/MainForm.cs
//
namespace MaintenanceToolGUI {
    public partial class MainForm : Form {
        :
        private void buttonDFU_Click(object sender, EventArgs e)
        {
            // ファームウェア更新画面を表示
            DFUForm f = new DFUForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // ファームウェア更新画面でCancelの場合は終了
                return;
            }
            :
            // ファームウェア更新
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_DFU)) {
                toolBLEDFU.DoCommandBLEDFU();
            }
            :
        }
```

`DoCommandBLEDFU`が呼び出されると、事前にBLE経由でバージョン情報を取得します。

```
//
// MaintenanceToolGUI/ToolBLEDFU.cs
//
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        public void DoCommandBLEDFU()
        {
            // ステータスを更新
            Status = BLEDFUStatus.GetCurrentVersion;

            // 認証器に導入中のバージョンを、BLE経由で照会
            // --> NotifyFirmwareVersionResponse が呼び出される
            bleMain.DoGetVersionInfoForDFU(this);
        }

        public void NotifyFirmwareVersionResponse(string strFWRev, string strHWRev)
        {
            // 認証器に導入中のバージョン、基板名を保持
            CurrentVersion = strFWRev;
            CurrentBoardname = strHWRev;
            :
            // 現在バージョン照会の場合（処理開始画面の表示前）
            if (Status == BLEDFUStatus.GetCurrentVersion) {
                // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
                ResumeCommandDFU();
            }
        }
```

バージョン情報が応答されると、認証器のバージョンが、管理ツールに同梱されているファームウェア更新イメージのバージョンより古いことを確認します。<br>
その後、処理開始画面（下図ご参照）がモーダル表示されます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        public void ResumeCommandDFU()
        {
            // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
            if (ReadDFUImageFile() == false) {
            ：
            // バージョンチェックが不正の場合は処理を終了
            if (DfuImageIsAvailable() == false) {
            ：
            // 処理開始画面を表示
            if (startForm.OpenForm(mainForm, this)) {
            ：
        }
```

処理開始画面（`BLEDFUStartForm`）は下図のようなイメージになります。

<img src="../MaintenanceTool/WindowsExe/assets06/0015.jpg" width="350">

#### 処理開始画面〜処理進捗画面の表示

処理開始画面上でOKボタンをクリックすると、処理開始画面が自動的に閉じられたのち、DFU処理クラスのメソッド`DoProcessDFU`が呼び出されます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        public void ResumeCommandDFU()
        {
            :
            // 処理開始画面を表示
            if (startForm.OpenForm(mainForm, this)) {
                // 処理開始画面でOKクリック-->DFU接続成功の場合、
                // DFU主処理開始
                DoProcessDFU();
                :
        }
```

DFU処理クラスは、処理進捗画面（下図ご参照）をモーダル表示し、DFU処理を別スレッドで開始させます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        private void DoProcessDFU()
        {
            // DFU主処理を起動
            Task task = Task.Run(() => {
                InvokeDFUProcess();
            });

            // 処理進捗画面を表示
            DialogResult ret = processingForm.OpenForm(mainForm);
            :
        }
```

処理進捗画面（`BLEDFUProcessingForm`）は下図のようなイメージになります。

<img src="../MaintenanceTool/WindowsExe/assets06/0016.jpg" width="350">

#### 更新イメージ転送開始

ファームウェア更新イメージは、BLE SMPサービスを使用して送信します。

BLE SMPサービスの処理は、クラス`ToolBLEDFUProcess`に実装されています。<br>
`InvokeDFUProcess`では、`ToolBLEDFUProcess`のエントリーポイント`PerformDFU`を呼び出します

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        private void InvokeDFUProcess()
        {
            // ステータスを更新
            Status = BLEDFUStatus.UploadProcess;
            :
            // DFU主処理を開始
            toolDFUProcess.PerformDFU();
        }
```

`PerformDFU`では、まずBLE SMPサービスに接続する処理を実行します。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFUProcess {
        :
        public void PerformDFU()
        {
            :
            // BLE SMPサービスに接続
            DoConnect();
        }

        private void OnConnected()
        {
            // スロット照会実行からスタート
            DoRequestGetSlotInfo();
        }
```

BLE SMPサービスに接続する（＝`OnConnected`が呼び出される）と、最初にスロット照会を実行し、スロット（認証器内部のFlash ROM領域）に関する情報を取得します。<br>
取得したスロット情報をチェックし、OKであれば、転送処理が開始されます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFUProcess {
        :
        private void DoRequestGetSlotInfo()
        {
            :
            // リクエストデータを送信
            Command = BLEDFUCommand.GetSlotInfo;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseGetSlotInfo(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            if (CheckSlotInfo(responseData) == false) {
            :
            // 転送元データを抽出
            ImageDataTotal = ToolBLEDFUImageRef.NRF53AppBin.Take(ToolBLEDFUImageRef.NRF53AppBinSize).ToArray();
            :
            // 転送処理に移行
            DoRequestUploadImage();
        }
```

#### 更新イメージ転送

転送処理では、ファームウェア更新イメージを240バイト前後のフレームに分割して送信します。[注1]<br>
同時に、処理進捗画面のプログレスバーを更新するための進捗値（0〜125）を、処理進捗画面に通知します。[注2]

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFUProcess {
        :
        private void DoRequestUploadImage()
        {
            // リクエストデータを生成
            byte[] bodyBytes = GenerateBodyForRequestUploadImage();
            :
            // リクエストデータを送信
            Command = BLEDFUCommand.UploadImage;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseUploadImage(byte[] responseData)
        {
            :
            // 転送比率を計算
            int imageBytesTotal = ToolBLEDFUImageRef.NRF53AppBinSize;
            int percentage = ImageBytesSent * 100 / imageBytesTotal;
            AppCommon.OutputLogDebug(string.Format("DFU image sent {0} bytes ({1}%)", ImageBytesSent, percentage));

            // 転送状況を画面表示
            string progressMessage = string.Format(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
            OnNotifyDFUProgress(progressMessage, percentage);

            // イメージ全体が転送されたかどうかチェック
            if (ImageBytesSent < imageBytesTotal) {
                :
                // 転送処理を続行
                DoRequestUploadImage();

            } else {
                :
                // 反映要求に移行
                DoRequestChangeImageUpdateMode();
            }
        }
```

処理進捗画面は、進捗通知（`OnNotifyDFUProgress`）により、画面上の進捗メッセージを切り替えます。<br>
下図は、ファームウェア更新イメージ転送中の様子です。

<img src="../MaintenanceTool/WindowsExe/assets06/0016.jpg" width="350">

[注1] １回あたりの転送バイト数は、下位関数`GenerateBodyForRequestUploadImage`内部で、データ本体が240バイトに収まるよう計算されます。<br>
[注2] 進捗値通知のステップは、転送率（0-100）の通知と、転送完了後に行われる反映待ち経過秒数（25秒間）の通知に分かれています。すなわち、転送中は進捗値が0〜100で推移し、転送後の反映待ちでは進捗値が101〜125で推移します。

#### 更新イメージ転送完了

ファームウェア更新イメージが全て転送されると、反映要求が実行されます。<br>
レスポンスのスロット情報をチェックし、OKであれば、ファームウェア更新イメージを反映させるためのリセット要求が実行されます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFUProcess {
        :
        private void DoRequestChangeImageUpdateMode()
        {
            :
            // リクエストデータを送信
            Command = BLEDFUCommand.ChangeImageUpdateMode;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseChangeImageUpdateMode(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            if (CheckUploadedSlotInfo(responseData) == false) {
            :
            // リセット要求に移行
            DoRequestResetApplication();
        }
```

リセット要求を実行すると、ファームウェアが再始動-->転送イメージの反映処理が開始されます。[注3]<br>
それと並行し、管理ツール側は転送イメージの反映待ちに移行するため、`ToolBLEDFUProcess`は`ToolBLEDFU`に制御を戻します。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFUProcess {
        :
        private void DoRequestResetApplication()
        {
            :
            // リクエストデータを送信
            Command = BLEDFUCommand.ResetApplication;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseResetApplication(byte[] responseData)
        {
            // DFU主処理の正常終了を通知
            TerminateDFUProcess(true);
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            OnTerminatedDFUProcess(success);
        }

namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        private void OnTerminatedDFUProcess(bool success)
        {
            :
            if (success) {
                // ステータスを更新（DFU反映待ち）
                Status = BLEDFUStatus.WaitForBoot;

                // DFU反映待ち処理を起動
                PerformDFUUpdateMonitor();
                :
        }
```

管理ツール側はファームウェア更新イメージの反映待ち（25秒間の待機状態）に入ります。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        private void PerformDFUUpdateMonitor()
        {
            :
            // 反映待ち（リセットによるファームウェア再始動完了まで待機）
            for (int i = 0; i < DFU_WAITING_SEC_ESTIMATED; i++) {
                // 処理進捗画面に通知
                OnNotifyDFUProgress(ToolGUICommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100 + i);
                System.Threading.Thread.Sleep(1000);
            }

            // 処理進捗画面に通知（DialogResult.OKで画面を閉じるよう指示）
            processingForm.NotifyTerminateDFUProcess(true);
        }
```

下図は、ファームウェア更新が反映されるのを待機している様子です。

<img src="../MaintenanceTool/WindowsExe/assets06/0017.jpg" width="350">

[注3] ファームウェアが再始動すると、BLE接続は切断されてしまいますが、Windows環境ではBLE切断時、再接続が自動的に行われるため、管理ツール側ではBLE再接続処理を実行しません。

#### 更新イメージ反映完了

25秒間の待機後は、自動的に処理進捗画面がクローズされます。<br>
その直後、DFU処理クラスは、BLE経由でバージョン情報照会を実行し、現在バージョン（FIDO認証器上に反映されているバージョン）を問い合わせます。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        private void DoProcessDFU()
        {
            :
            // 処理進捗画面を表示
            DialogResult ret = processingForm.OpenForm(mainForm);
            :
            if (ret == DialogResult.OK) {
	            // ステータスを更新（バージョン更新判定）
	            Status = BLEDFUStatus.CheckUpdateVersion;

                // 認証器に導入された更新バージョンを、BLE経由で照会
                // --> NotifyFirmwareVersionResponse が呼び出される
                bleMain.DoGetVersionInfoForDFU(this);
                :
        }
```


FIDO認証器からバージョン情報が応答されると、FIDO認証器に転送したファームウェアのバージョン（`UpdateVersion`）と、現在バージョン（`CurrentVersion`）を比較します。<br>
比較結果、バージョンが同じであれば反映確認OKと判定し、DFU処理が正常終了します。

```
namespace MaintenanceToolGUI {
    public class ToolBLEDFU {
        :
        public void NotifyFirmwareVersionResponse(string strFWRev, string strHWRev)
        {
            // 認証器に導入中のバージョン、基板名を保持
            CurrentVersion = strFWRev;
            CurrentBoardname = strHWRev;

            // バージョン更新判定の場合（ファームウェア反映待ち）
            if (Status == BLEDFUStatus.CheckUpdateVersion) {
                :
                // バージョン情報を比較して終了判定
                // --> 判定結果をメイン画面に戻す
                mainForm.OnAppMainProcessExited(CompareUpdateVersion());
                return;
            }
            :
        }

        private bool CompareUpdateVersion()
        {
            // バージョン情報を比較
            bool versionEqual = (CurrentVersion == UpdateVersion);
            if (versionEqual) {
                // バージョンが同じであればDFU処理は正常終了
            :
            // メイン画面に制御を戻す
            return versionEqual;
        }
```

<img src="../MaintenanceTool/WindowsExe/assets06/0018.jpg" width="350">

## プログラムの仕様

プログラムの仕様に関する情報を掲載いたします。

#### 処理性能について

処理条件<br>
・転送先基板＝nRF5340 DK（`PCA10095`）<br>
・転送ファームウェア＝`Version 0.4.4`

| # |項目 |内容 |
|:-:|:-|:-|
|1|所要時間|転送前処理＝約`10`秒、転送＝約`50`秒、反映＝約`25`秒|
|2|転送速度|約`4,888 bytes/sec`|
|3|転送許容サイズ|約`245,500 bytes`|

#### DFUの流れについて

DFU機能は、以下の流れで行われます。

| # |項目 |処理内容 |
|:-:|:-|:-|
|1|バージョン情報照会|認証器ファームウェアのバージョンを、BLE経由で照会します。|
|2|DFUイメージ抽出|ファームウェア更新イメージファイルを読込み、転送DFUイメージを<br>抽出します。<br>また、更新イメージファイルのバージョンをファイル名から取得します。<br>（ファイル名＝`app_update.<基板名>.<バージョン文字列>.bin`）|
|3|バージョンチェック|以下のケースではDFU処理を行わないようにします。<br>・認証器ファームウェアの現在バージョンが、更新イメージファイルの<br>バージョンより新しい場合<br>・認証器ファームウェアの現在バージョンが、`0.4.0`より古い場合|
|4|DFU処理開始指示|サブスレッド上で行われます。|
|5|DFU対象デバイスに接続|認証器に、BLE経由で接続します。|
|6|DFUトランザクション実行|認証器に、前述のファームウェア更新イメージを転送します。|
|7|反映待機|DFUイメージ転送完了後、認証器ファームウェアが自動起動し、再度<br>BLE接続が可能になるまで、`25`秒間待機します。[注4]|
|8|バージョン情報照会|認証器と再度BLE接続し、BLE経由でバージョン情報照会を実行します。|
|9|バージョンチェック|バージョン情報照会で取得したバージョン文字列をチェックし、<br>イメージファイル名から取得した更新バージョン文字列と等しければ、<br>DFU処理は完了となります。|


前述の `6`「DFUトランザクション実行」は、以下の流れで行われます。[注5]

| # |コマンド |処理内容 |
|:-:|:-|:-|
|1|スロット情報照会|スロット（認証器のFlash ROM領域）の情報を取得します。|
|2|ファームウェア更新イメージ転送|Flash ROMに配置されるプログラムイメージを転送します。|
|3|反映要求|転送されたファームウェアをリセット後に反映させるよう、<br>BLE SMPサービスを経由し、認証器ファームウェアに指示<br>します。|
|4|リセット要求|転送されたファームウェアを反映させるために、リセット<br>（再始動）を認証器ファームウェアに指示します。|

[注4] 認証器ファームウェアとのBLE再接続は、Windowsにより自動的に行われます。<br>
[注5] DFU処理中は処理進捗画面をモーダル表示させている（＝メインスレッドが停止している）ため、別スレッドで実行されるDFU処理のタイムアウト監視を、メインスレッドから実行することができません。その代わりに、DFUトランザクション内で実行されるコマンド（表の各項目）ごとに、BLE応答タイムアウト監視（`10`秒）を行うようにしています。

#### [DFUトランザクションの内容](../nRF5340_app/BLEDFU_TRANSACTION.md)

DFUの処理において受け渡しされるデータ（DFUトランザクションデータ）について掲載しています。
