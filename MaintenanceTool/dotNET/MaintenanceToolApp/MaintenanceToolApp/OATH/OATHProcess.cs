﻿using MaintenanceToolApp;
using System.Windows;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.AppDefine.Command;
using static MaintenanceToolApp.AppDefine.Transport;

namespace MaintenanceTool.OATH
{
    public class OATHParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public Transport Transport { get; set; }

        public OATHParameter()
        {
            CommandTitle = string.Empty;
            Command = COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            Transport = TRANSPORT_NONE;
        }
    }

    public class OATHProcess
    {
        // 処理実行のためのプロパティー
        private readonly OATHParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public OATHProcess(OATHParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case COMMAND_OATH_SCAN_QRCODE:
                // QRコードスキャン-->アカウント登録-->ワンタイムパスワード参照を一息で実行
                new ScanQRCodeWindow().ShowDialogWithOwner(ParentWindow);
                break;
            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowWarningMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }
    }
}
