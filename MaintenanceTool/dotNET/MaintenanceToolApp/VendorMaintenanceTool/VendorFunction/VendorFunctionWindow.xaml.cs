using MaintenanceToolApp;
using System.Windows;
using ToolAppCommon;

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

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
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

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
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

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
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

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
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
