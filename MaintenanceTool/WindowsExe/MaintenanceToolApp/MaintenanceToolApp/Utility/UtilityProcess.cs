using MaintenanceToolApp.CommonWindow;
using System;
using System.Diagnostics;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.Utility
{
    public class UtilityParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }

        public UtilityParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
        }

        public override string ToString()
        {
            return string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
        }
    }

    public class UtilityProcess
    {
        // 処理実行のためのプロパティー
        private readonly UtilityParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        // HIDインターフェースからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public UtilityProcess(UtilityParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_RTCC_SETTING:
                // メイン画面を親ウィンドウとし、時刻設定画面を開く
                new RTCCSettingWindow().ShowDialogWithOwner(ParentWindow);
                break;

            case Command.COMMAND_HID_GET_FLASH_STAT:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_GET_FLASH_STAT;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);

                // Flash ROM情報照会を実行
                DoRequestHIDGetFlashStat();
                break;

            case Command.COMMAND_HID_GET_VERSION_INFO:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_GET_VERSION_INFO;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);

                // バージョン情報取得処理を実行
                DoRequestHIDGetVersionInfo();
                break;

            case Command.COMMAND_VIEW_APP_VERSION:
                // メイン画面を親ウィンドウとし、バージョン参照画面を開く
                CommonVersionWindow w = new CommonVersionWindow();
                w.ShowDialogWithOwner(ParentWindow);
                break;

            case Command.COMMAND_VIEW_LOG_FILE:
                // 管理ツールのログファイルを格納している
                // フォルダーを、Windowsのエクスプローラで参照
                ViewLogFile();
                break;

            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        //
        // 内部処理
        //
        private void DoRequestHIDGetFlashStat()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCommand(HIDProcessConst.HID_CMD_GET_FLASH_STAT, new byte[0]);
        }

        private void DoResponseHIDGetFlashStat(byte[] responseData)
        {
            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                // エラーの場合は画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                return;
            }
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppUtil.ExtractCBORBytesFromResponse(responseData, responseData.Length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);
            AppLogUtil.OutputLogDebug("Flash ROM statistics: " + responseCSV);

            // 情報取得CSVから空き領域に関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strUsed = "";
            string strAvail = "";
            string strCorrupt = "";
            foreach (string v in vars) {
                if (v.StartsWith("words_used=")) {
                    strUsed = v.Split('=')[1];
                } else if (v.StartsWith("words_available=")) {
                    strAvail = v.Split('=')[1];
                } else if (v.StartsWith("corruption=")) {
                    strCorrupt = v.Split('=')[1];
                }
            }

            // 空き容量、破損状況を画面に表示
            string rateText;
            if (strUsed.Length > 0 && strAvail.Length > 0) {
                float avail = float.Parse(strAvail);
                float remaining = avail - float.Parse(strUsed);
                float rate = remaining / avail * 100;
                rateText = string.Format(AppCommon.MSG_FSTAT_REMAINING_RATE, rate);
            } else {
                rateText = AppCommon.MSG_FSTAT_NON_REMAINING_RATE;
            }
            string corruptText = (strCorrupt.Equals("0")) ?
                AppCommon.MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : AppCommon.MSG_FSTAT_CORRUPTING_AREA_EXIST;

            // 画面に制御を戻す
            CommandProcess.NotifyMessageToMainUI(string.Format("  {0}{1}", rateText, corruptText));
            CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, "", true, ParentWindow);
        }

        private void DoRequestHIDGetVersionInfo()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCommand(0x80 | MNT_COMMAND_BASE, new byte[] { MNT_COMMAND_GET_APP_VERSION });
        }

        private void DoResponseHIDGetVersionInfo(byte[] responseData)
        {
            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                // エラーの場合は画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                return;
            }

            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppUtil.ExtractCBORBytesFromResponse(responseData, responseData.Length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);

            // 情報取得CSVからバージョン情報を抽出
            string[] array = ExtractValuesFromVersionInfo(responseCSV);
            string strDeviceName = array[0];
            string strFWRev = array[1];
            string strHWRev = array[2];
            string strSecic = array[3];

            // 画面に制御を戻す
            CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_VERSION_INFO_HEADER);
            CommandProcess.NotifyMessageToMainUI(string.Format(AppCommon.MSG_VERSION_INFO_DEVICE_NAME, strDeviceName));
            CommandProcess.NotifyMessageToMainUI(string.Format(AppCommon.MSG_VERSION_INFO_FW_REV, strFWRev));
            CommandProcess.NotifyMessageToMainUI(string.Format(AppCommon.MSG_VERSION_INFO_HW_REV, strHWRev));

            // セキュアICの搭載有無を表示
            if (strSecic.Length > 0) {
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_VERSION_INFO_SECURE_IC_AVAIL);
            } else {
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_VERSION_INFO_SECURE_IC_UNAVAIL);
            }

            // 画面に制御を戻す
            CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, "", true, ParentWindow);
        }

        private string[] ExtractValuesFromVersionInfo(string responseCSV)
        {
            // 情報取得CSVからバージョンに関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strDeviceName = "";
            string strFWRev = "";
            string strHWRev = "";
            string strSecic = "";
            foreach (string v in vars) {
                if (v.StartsWith("DEVICE_NAME=")) {
                    strDeviceName = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("FW_REV=")) {
                    strFWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("HW_REV=")) {
                    strHWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("ATECC608A=")) {
                    strSecic = v.Split('=')[1].Replace("\"", "");
                }
            }
            return new string[] { strDeviceName, strFWRev, strHWRev, strSecic };
        }

        private void ViewLogFile()
        {
            // 管理ツールのログファイルを格納している
            // フォルダーを、Windowsのエクスプローラで参照
            try {
                var procInfo = new ProcessStartInfo {
                    FileName = AppLogUtil.OutputLogFileDirectoryPath(),
                    UseShellExecute = true
                };
                Process.Start(procInfo);

            } catch (Exception e) {
                AppLogUtil.OutputLogError(String.Format(AppCommon.MSG_FORMAT_UTILITY_VIEW_LOG_FILE_ERR, e.Message));
            }
        }

        //
        // HIDからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success, ParentWindow);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_HID_GET_FLASH_STAT:
                DoResponseHIDGetFlashStat(responseData);
                break;
            case Command.COMMAND_HID_GET_VERSION_INFO:
                DoResponseHIDGetVersionInfo(responseData);
                break;
            default:
                // メイン画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                break;
            }
        }
    }
}
