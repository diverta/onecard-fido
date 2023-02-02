using MaintenanceToolApp;
using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using VendorMaintenanceTool.FIDOSettings;
using static MaintenanceToolApp.AppDefine;

namespace VendorMaintenanceTool.VendorFunction
{
    /// <summary>
    /// VendorFunctionWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class VendorFunctionWindow : Window
    {
        public VendorFunctionWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、オーナー画面の中央にモード付きで表示
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
        // 業務処理
        //
        private void DoInstallAttestation()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 鍵・証明書インストール画面をポップアップ表示
            VendorFunctionParameter param = new VendorFunctionParameter();
            FIDOAttestationWindow w = new FIDOAttestationWindow(param);
            if (w.ShowDialogWithOwner(this) == false) {
                return;
            }

            // 鍵・証明書インストール
            DoVendorFunctionProcess(param, Command.COMMAND_INSTALL_SKEY_CERT, VendorAppCommon.PROCESS_NAME_INSTALL_ATTESTATION);
        }

        private void DoRemoveAttestation()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 確認メッセージを表示し、Yesの場合だけ処理を続行する
            string message = string.Format("{0}\n\n{1}",
                VendorAppCommon.MSG_ERASE_SKEY_CERT, VendorAppCommon.MSG_PROMPT_ERASE_SKEY_CERT);
            if (DialogUtil.DisplayPromptPopup(this, VendorAppCommon.PROCESS_NAME_REMOVE_ATTESTATION, message) == false) {
                return;
            }

            // 鍵・証明書の削除
            DoVendorFunctionProcess(new VendorFunctionParameter(), Command.COMMAND_ERASE_SKEY_CERT, VendorAppCommon.PROCESS_NAME_REMOVE_ATTESTATION);
        }

        private void DoBootloaderMode()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 確認メッセージを表示し、Yesの場合だけ処理を続行する
            string message = string.Format("{0}\n\n{1}",
                VendorAppCommon.MSG_CHANGE_TO_BOOTLOADER_MODE, VendorAppCommon.MSG_PROMPT_CHANGE_TO_BOOTLOADER_MODE);
            if (DialogUtil.DisplayPromptPopup(this, VendorAppCommon.PROCESS_NAME_BOOT_LOADER_MODE, message) == false) {
                return;
            }

            // ブートローダーモード遷移
            DoVendorFunctionProcess(new VendorFunctionParameter(), Command.COMMAND_HID_BOOTLOADER_MODE, VendorAppCommon.PROCESS_NAME_BOOT_LOADER_MODE);
        }

        private void DoFirmwareReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 確認メッセージを表示し、Yesの場合だけ処理を続行する
            string message = string.Format("{0}\n\n{1}",
                VendorAppCommon.MSG_FIRMWARE_RESET, VendorAppCommon.MSG_PROMPT_FIRMWARE_RESET);
            if (DialogUtil.DisplayPromptPopup(this, VendorAppCommon.PROCESS_NAME_FIRMWARE_RESET, message) == false) {
                return;
            }

            // 認証器のファームウェア再起動
            DoVendorFunctionProcess(new VendorFunctionParameter(), Command.COMMAND_HID_FIRMWARE_RESET, VendorAppCommon.PROCESS_NAME_FIRMWARE_RESET);
        }

        //
        // コマンド実行指示～完了後の処理
        //
        private void DoVendorFunctionProcess(VendorFunctionParameter param, Command command, string commandTitle)
        {
            // 実行コマンド／コマンド名称を設定
            param.Command = command;
            param.CommandTitle = commandTitle;

            Task task = Task.Run(() => {
                // コマンドを実行
                new VendorFunctionProcess().DoProcess(param, OnVendorFunctionProcessTerminated);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);

            // メッセージをポップアップ表示
            if (param.CommandSuccess) {
                DialogUtil.ShowInfoMessage(this, Title, param.ResultMessage);
            } else {
                DialogUtil.ShowWarningMessage(this, param.ResultMessage, param.ResultInformativeMessage);
            }
        }

        private void OnVendorFunctionProcessTerminated()
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonInstallAttestation_Click(object sender, RoutedEventArgs e)
        {
            DoInstallAttestation();
        }

        private void buttonRemoveAttestation_Click(object sender, RoutedEventArgs e)
        {
            DoRemoveAttestation();
        }

        private void buttonBootloaderMode_Click(object sender, RoutedEventArgs e)
        {
            DoBootloaderMode();
        }

        private void buttonFirmwareReset_Click(object sender, RoutedEventArgs e)
        {
            DoFirmwareReset();
        }
    }
}
