﻿using System;
using System.Diagnostics;
using System.IO;

namespace U2FMaintenanceToolGUI
{
    class AppMain
    {
        public const string U2FMaintenanceToolTitle = "U2F Maintenance Tool";
        private const string U2FMaintenanceToolExe = "U2FMaintenanceTool.exe";
        private bool commandAvailable;

        public AppMain()
        {
            // U2F管理ツール（CUI版）が導入されているかチェック
            commandAvailable = File.Exists(U2FMaintenanceToolExe);
            if (commandAvailable == false) {
                outputLogToFile(U2FMaintenanceToolExe + "が導入されていません");
            }
            outputLogToFile("U2F管理ツールを起動しました");
        }

        private void outputLogToFile(string message)
        {
            // メッセージに現在時刻を付加する
            string formatted = string.Format("{0} {1}", DateTime.Now.ToString(), message);

            // ログファイルにメッセージを出力する
            string fname = "U2FMaintenanceToolGUI.log";
            StreamWriter sr = new StreamWriter(
                (new FileStream(fname, FileMode.Append)), 
                System.Text.Encoding.Default);
            sr.WriteLine(formatted);
            sr.Close();
        }

        private bool doCommandWithExecutable(string executable, string arguments)
        {
            // 実行対象プロセスの実行可能ファイルがない場合は終了
            if (commandAvailable == false) {
                outputLogToFile(string.Format(
                    "コマンドの実行が出来ません: {0} {1}",
                    executable, arguments));
                return false;
            }

            // MS-DOSコマンドプロンプト画面が表示されないように
            // プロセスを実行する
            ProcessStartInfo psInfo = new ProcessStartInfo() {
                FileName = executable,
                Arguments = arguments,
                CreateNoWindow = true,
                UseShellExecute = false
            };

            // プロセス実行が完了するまで待つ
            Process p = Process.Start(psInfo);
            p.WaitForExit();

            // 実行結果をログ出力
            bool ret = (p.ExitCode == 0);
            outputLogToFile(string.Format(
                "コマンドの実行が{0}しました: {1} {2}",
                ret ? "成功" : "失敗", executable, arguments));
            return ret;
        }

        public bool doEraseBond()
        {
            // U2FMaintenanceTool.exe -Bを実行する
            return doCommandWithExecutable(U2FMaintenanceToolExe, "-B");
        }

        public bool doEraseSkeyCert()
        {
            // U2FMaintenanceTool.exe -Eを実行する
            return doCommandWithExecutable(U2FMaintenanceToolExe, "-E");
        }

        public bool doInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // U2FMaintenanceTool.exe -I <skeyFilePath> <certFilePath> を実行する
            string arguments = string.Format("-I {0} {1}", skeyFilePath, certFilePath);
            return doCommandWithExecutable(U2FMaintenanceToolExe, arguments);
        }

        public bool doHealthCheck()
        {
            // U2FMaintenanceTool.exe -Hを実行する
            return doCommandWithExecutable(U2FMaintenanceToolExe, "-H");
        }

        public bool doSetupChromeNativeMessaging()
        {
            // U2FMaintenanceTool.exe -Rを実行する
            return doCommandWithExecutable(U2FMaintenanceToolExe, "-R");
        }

        public void doExit()
        {
            System.Windows.Forms.Application.Exit();
            outputLogToFile("U2F管理ツールを終了しました");
        }
    }
}
