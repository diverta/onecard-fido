using System;
using System.Diagnostics;
using System.IO;

namespace U2FHelper
{
    class U2FMaintenanceCommand
    {
        // U2F管理コマンドの情報
        public const string U2FMaintenanceCmdExe = AppCommon.FILENAME_U2FCOMMAND_EXE;
        public bool commandAvailable;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行中の外部プロセスを保持
        private Process p;

        public U2FMaintenanceCommand(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // U2F管理コマンドが導入されているかチェック
            commandAvailable = File.Exists(U2FMaintenanceCmdExe);
            if (commandAvailable == false) {
                outputLogToFile(string.Format(AppCommon.MSG_FORMAT_NOT_INSTALLED, U2FMaintenanceCmdExe));
            }
        }

        private void outputLogToFile(string message)
        {
            // メッセージに現在時刻を付加する
            string formatted = string.Format("{0} {1}", DateTime.Now.ToString(), message);

            // ログファイルにメッセージを出力する
            AppCommon.OutputLogText(formatted);
        }

        private void doCommandWithExecutable(string executable, string arguments)
        {
            // 実行対象プロセスの実行可能ファイルがない場合は終了
            if (commandAvailable == false) {
                return;
            }

            // MS-DOSコマンドプロンプト画面が表示されないように
            // プロセスを実行する
            p = new Process();
            p.OutputDataReceived += processOutputDataReceived;
            p.ErrorDataReceived += processErrorDataReceived;

            p.StartInfo.FileName = executable;
            p.StartInfo.Arguments = arguments;
            p.StartInfo.CreateNoWindow = true;
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.RedirectStandardError = true;

            // イベントハンドラーがフォームを作成したスレッドで実行されるようにする
            p.SynchronizingObject = mainForm;
            // イベントハンドラー追加
            p.Exited += new EventHandler(onProcessExited);
            // プロセスが終了したときにExitedイベントを発生させる
            p.EnableRaisingEvents = true;

            // プロセス実行が完了するまで待つ
            p.Start();
            p.BeginOutputReadLine();
            p.BeginErrorReadLine();

            // 実行開始ログ出力
            outputLogToFile(string.Format(
                AppCommon.MSG_FORMAT_PROCESS_STARTED, 
                AppCommon.MSG_U2FCOMMAND_PROCESS,
                p.StartInfo.FileName, p.StartInfo.Arguments));
        }

        public bool commandProcessRunning()
        {
            // プロセスがない場合
            if (p == null) {
                return false;
            }
            // プロセスが既に停止している場合
            if (p.HasExited == true) {
                return false;
            }
            // プロセスが走行中と判定
            return true;
        }

        private void onProcessExited(object sender, EventArgs e)
        {
            // プロセスのリターンコードを取得
            bool ret = (p.ExitCode == 0);

            // 実行結果をログ出力
            outputLogToFile(string.Format(
                AppCommon.MSG_FORMAT_PROCESS_EXITED,
                AppCommon.MSG_U2FCOMMAND_PROCESS,
                ret ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE,
                p.StartInfo.FileName, p.StartInfo.Arguments));

            // コマンド実行完了時の処理を実行
            mainForm.OnU2FCommandProcessExited(ret);
        }

        private void processOutputDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            // メイン画面の参照を経由し処理を実行
            mainForm.OnU2FCommandProcessOutputData(args.Data);
        }

        private void processErrorDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            // メイン画面の参照を経由し処理を実行
            mainForm.OnU2FCommandProcessErrorData(args.Data);
        }

        public void DoXferMessage( byte[] message, int length)
        {
            // 転送したいメッセージをBase64エンコード
            byte[] xferMessage = new byte[length];
            for (int i = 0; i < length; i++) {
                xferMessage[i] = message[i];
            }
            string encodedText = Convert.ToBase64String(xferMessage);

            // web-safe形式に変換
            encodedText = encodedText.Replace('/', '_');
            encodedText = encodedText.Replace('+', '-');

            // U2FMaintenanceTool.exe -Xを実行する
            string arguments = string.Format("-X \"{0}\"", encodedText);
            doCommandWithExecutable(U2FMaintenanceCmdExe, arguments);
        }
    }
}
