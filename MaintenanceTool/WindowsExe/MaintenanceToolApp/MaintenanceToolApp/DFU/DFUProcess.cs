﻿using System.Windows;

namespace MaintenanceToolApp.DFU
{
    public class DFUParameter
    {
        public string CurrentVersion { get; set; }
        public string UpdateVersion { get; set; }

        public DFUParameter()
        {
            CurrentVersion = string.Empty;
            UpdateVersion = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("CurrentVersion:{0} UpdateVersion:{1}", CurrentVersion, UpdateVersion);
        }
    }

    public class DFUProcess
    {
        // 処理実行のためのプロパティー
        private readonly DFUParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public DFUProcess(DFUParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
        }

        //
        // 処理開始前の確認
        //
        public static bool ConfirmDoProcess(Window currentWindow)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_PROMPT_START_BLE_DFU_PROCESS,
                AppCommon.MSG_COMMENT_START_BLE_DFU_PROCESS);

            // プロンプトを表示し、Yesの場合だけ処理を続行する
            return DialogUtil.DisplayPromptPopup(currentWindow, AppCommon.MSG_TOOL_TITLE, message);
        }
    }
}
