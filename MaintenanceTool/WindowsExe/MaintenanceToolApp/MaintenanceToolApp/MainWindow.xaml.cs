﻿using MaintenanceToolApp.CommonProcess;
using MaintenanceToolApp.CommonWindow;
using MaintenanceToolApp.DFU;
using MaintenanceToolApp.HealthCheck;
using MaintenanceToolApp.Utility;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            // メイン画面のタイトルを設定
            Title = AppCommon.MSG_TOOL_TITLE;

            // アプリケーション開始ログを出力
            AppLogUtil.SetOutputLogApplName("MaintenanceToolApp");
            AppLogUtil.OutputLogInfo(string.Format("{0}を起動しました: {1}", AppCommon.MSG_TOOL_TITLE, AppUtil.GetAppVersionString()));

            // USBデバイスの脱着検知を開始
            USBDevice.StartUSBDeviceNotification(this);

            // USBデバイスの接続試行
            HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDevice);
            HIDProcess.ConnectHIDDevice();

            // 機能クラスからのコールバックを登録
            CommandProcess.RegisterHandlerOnEnableButtonsOfMainUI(EnableButtons);
            CommandProcess.RegisterHandlerOnNotifyMessageToMainUI(AppendMessageText);
        }

        void OnConnectHIDDevice(bool connected)
        {
            // 接続／切断検知結果をテキストボックス上に表示
            string messageText = connected ? AppCommon.MSG_HID_CONNECTED : AppCommon.MSG_HID_REMOVED;
            AppendMessageText(messageText);
        }

        void EnableButtons(bool enable)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonBLE.IsEnabled = enable;
            buttonFIDO.IsEnabled = enable;
            buttonDFU.IsEnabled = enable;
            buttonSetPivParam.IsEnabled = enable;
            buttonSetPgpParam.IsEnabled = enable;
            buttonOATH.IsEnabled = enable;
            buttonQuit.IsEnabled = enable;
            buttonHealthCheck.IsEnabled = enable;
            buttonUtility.IsEnabled = enable;
        }

        void AppendMessageText(string messageText)
        {
            // 引数の文字列を、テキストボックス上に表示し改行
            textBoxMessage.Text += messageText + "\r\n";

            // テキストボックスの現在位置を末尾に移動
            textBoxMessage.ScrollToEnd();
        }

        private void DoDFU()
        {
            // 処理前の確認
            if (DFUProcess.ConfirmDoProcess(this) == false) {
                return;
            }

            // コマンドを別スレッドで起動
            Task task = Task.Run(() => {
                // バージョン情報照会から開始
                VersionInfoProcess.DoRequestVersionInfo(DoDFUResume);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);
        }

        private void DoDFUResume(bool success, string errorMessage, VersionInfoData versionInfoData)
        {
            // 進捗画面を閉じる
            CommonProcessingWindow.NotifyTerminate();

            // バージョン情報照会失敗時は、以降の処理を実行しない
            if (success == false || versionInfoData == null) {
                DialogUtil.ShowWarningMessage(this, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_DFU_VERSION_INFO_GET_FAILED);
                return;
            }

            // ファームウェア更新画面を開き、実行を指示
            DFUParameter param = new DFUParameter(versionInfoData);
            bool b = new DFUWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // DFU機能を実行
                new DFUProcess(param).DoProcess();
            }
        }

        private void DoHealthCheck()
        {
            // ヘルスチェック実行画面を開き、実行コマンド種別を設定
            HealthCheckParameter param = new HealthCheckParameter();
            bool b = new HealthCheckWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // ユーティリティー機能を実行
                new HealthCheckProcess(param).DoProcess();
            }
        }

        private void DoUtility()
        {
            // ユーティリティー画面を開き、実行コマンド種別を設定
            UtilityParameter param = new UtilityParameter();
            bool b = new UtilityWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // ユーティリティー機能を実行
                new UtilityProcess(param).DoProcess();
            }
        }

        private void TerminateWindow()
        {
            // 画面を閉じる
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonHealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoHealthCheck();
        }

        private void buttonUtility_Click(object sender, RoutedEventArgs e)
        {
            DoUtility();
        }

        private void buttonQuit_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow();
        }

        private void buttonDFU_Click(object sender, RoutedEventArgs e)
        {
            DoDFU();
        }
    }
}
