using MaintenanceToolApp;
using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceTool.OATH
{
    internal class OATHWindowSwitcher
    {
        // 処理パラメーターの参照を保持
        private readonly OATHParameter Parameter = null!;

        public OATHWindowSwitcher(OATHParameter parameter)
        {
            // 処理パラメーターの参照を保持
            Parameter = parameter;
        }

        //
        // 画面表示制御
        //
        public void Switch(Window parentWindow)
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_OATH_SCAN_QRCODE:
                // QRコードスキャン-->アカウント登録-->ワンタイムパスワード参照を一息で実行
                new ScanQRCodeWindow(Parameter).ShowDialogWithOwner(parentWindow);
                break;
            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowWarningMessage(parentWindow, parentWindow.Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }
    }
}
