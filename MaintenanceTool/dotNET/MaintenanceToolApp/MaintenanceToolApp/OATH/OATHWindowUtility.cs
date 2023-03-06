using MaintenanceToolApp;
using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceTool.OATH
{
    internal class OATHWindowUtility
    {
        // 処理パラメーターの参照を保持
        private readonly OATHParameter Parameter = null!;

        public OATHWindowUtility(OATHParameter parameter)
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
            case Command.COMMAND_OATH_SHOW_PASSWORD:
                // ワンタイムパスワード参照画面を表示
                ShowTOTPDisplayWindow(parentWindow);
                break;
            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowWarningMessage(parentWindow, parentWindow.Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        private void ShowTOTPDisplayWindow(Window parentWindow)
        {
            // 選択されたアカウントをチェック
            string[] account = Parameter.SelectedAccount.Split(":");
            if (account.Length != 2) {
                return;
            }

            // ワンタイムパスワードを事前生成
            Parameter.OATHAccountIssuer = account[0];
            Parameter.OATHAccountName = account[1];
            Parameter.CommandTitle = AppCommon.MSG_LABEL_COMMAND_OATH_UPDATE_TOTP;
            if (DoOATHProcess(parentWindow, Parameter) == false) {
                return;
            }

            // ワンタイムパスワード参照画面を表示
            new TOTPDisplayWindow(Parameter).ShowDialogWithOwner(parentWindow);
        }

        //
        // OATHコマンド実行ユーティリティー
        //
        public static bool DoOATHProcess(Window window, OATHParameter parameter)
        {
            // パラメーターを設定し、コマンドを実行
            Task task = Task.Run(() => {
                new OATHProcess(parameter).DoProcess(OnOATHProcessTerminated);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(window);

            if (parameter.CommandSuccess == false) {
                // 処理失敗時は、エラーメッセージをポップアップ表示
                DialogUtil.ShowWarningMessage(window, parameter.ResultMessage, parameter.ResultInformativeMessage);
                return false;
            }

            return true;
        }

        private static void OnOATHProcessTerminated(OATHParameter parameter)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }
    }
}
