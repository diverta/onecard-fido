using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.Utility
{
    /// <summary>
    /// RTCCSettingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class RTCCSettingWindow : Window
    {
        // 時刻設定処理の参照を保持
        private readonly RTCCSettingProcess Process = null!;
        private readonly RTCCSettingParameter Parameter = null!;

        public RTCCSettingWindow()
        {
            // 時刻設定処理クラスの参照を保持
            Parameter = new RTCCSettingParameter();
            Process = new RTCCSettingProcess(Parameter);

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // 画面項目の初期化
            InitFieldValue();

            // ユーティリティー画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // ラジオボタン「USB経由」を選択状態にする
            buttonTransportUSB.IsChecked = true;

            // 時刻表示ラベルをブランクにする
            LabelToolTimestamp.Content = string.Empty;
            LabelDeviceTimestamp.Content = string.Empty;
        }

        private void DoSetParameter(Transport transport)
        {
            // トランスポート種別を設定
            Parameter.Transport = transport;
        }

        //
        // 時刻設定処理
        //
        private void DoSetTimestamp()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (DialogUtil.DisplayPromptPopup(this, AppCommon.MSG_PROMPT_RTCC_SET_TIMESTAMP, AppCommon.MSG_COMMENT_RTCC_SET_TIMESTAMP)) {
                DoRTCCSettingProcess(Command.COMMAND_RTCC_SET_TIMESTAMP, AppCommon.PROCESS_NAME_RTCC_SET_TIMESTAMP);
            }
        }

        private void DoGetTimestamp()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }
            DoRTCCSettingProcess(Command.COMMAND_RTCC_GET_TIMESTAMP, AppCommon.PROCESS_NAME_RTCC_GET_TIMESTAMP);
        }

        private void DoRTCCSettingProcess(Command command, string commandTitle)
        {
            // パラメーターを設定し、コマンドを実行
            Parameter.Command = command;
            Parameter.CommandTitle = commandTitle;
            Task task = Task.Run(() => {
                Process.DoRTCCSettingProcess(OnRTCCSettingProcessTerminated);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);

            // タイムスタンプを画面表示
            LabelToolTimestamp.Content = Parameter.ToolTimestamp;
            LabelDeviceTimestamp.Content = Parameter.DeviceTimestamp;

            // メッセージをポップアップ表示
            if (Parameter.CommandSuccess) {
                DialogUtil.ShowInfoMessage(this, Title, Parameter.ResultMessage);
            } else {
                DialogUtil.ShowWarningMessage(this, Parameter.ResultMessage, Parameter.ResultInformativeMessage);
            }
        }

        private void OnRTCCSettingProcessTerminated(RTCCSettingParameter param)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }

        //
        // イベント処理部
        // 
        private void buttonTransportUSB_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_HID);
        }

        private void buttonTransportBLE_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_BLE);
        }

        private void buttonGetTimestamp_Click(object sender, RoutedEventArgs e)
        {
            DoGetTimestamp();
        }

        private void buttonSetTimestamp_Click(object sender, RoutedEventArgs e)
        {
            DoSetTimestamp();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
