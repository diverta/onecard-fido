using System;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace U2FMaintenanceToolGUI
{
    class AppMain
    {
        // U2F管理コマンドの情報
        public const string U2FMaintenanceToolTitle = "U2F Maintenance Tool";
        public const string U2FMaintenanceToolExe = "U2FMaintenanceToolCMD.exe";
        public bool commandAvailable;

        // U2F管理コマンドからの出力を保持
        private static StringBuilder standardOutputs;
        private static StringBuilder standardErrors;

        // Chrome Native Messaging設定用の
        // レジストリーキー名、JSONファイル名
        public const string ChromeNMRegistryKey = "jp.co.diverta.chrome.helper.ble.u2f";
        public const string ChromeNMSettingFile = "jp.co.diverta.chrome.helper.ble.u2f.json";

        // OpenSSLコマンドの情報
        public const string OpenSSLExe = "openssl.exe";
        public const string OpenSSLCacertV3Ext = "cacertV3.ext";
        public bool opensslAvailable;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行中の外部プロセスを保持
        private Process p;

        public AppMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // U2F管理コマンドが導入されているかチェック
            commandAvailable = File.Exists(U2FMaintenanceToolExe);
            if (commandAvailable == false) {
                outputLogToFile(U2FMaintenanceToolExe + "が導入されていません");
            }
            // OpenSSLコマンドが導入されているかチェック
            if (File.Exists(OpenSSLExe) == false) {
                outputLogToFile(OpenSSLExe + "が導入されていません");
                opensslAvailable = false;
            } else if (File.Exists(OpenSSLCacertV3Ext) == false) {
                outputLogToFile(OpenSSLCacertV3Ext + "が導入されていません");
                opensslAvailable = false;
            } else {
                opensslAvailable = true;
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

        private void doCommandWithExecutable(string executable, string arguments)
        {
            // 実行対象プロセスの実行可能ファイルがない場合は終了
            if (commandAvailable == false) {
                return;
            }

            // プロセス出力情報をクリア
            standardOutputs = new StringBuilder();
            standardErrors = new StringBuilder();

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
                "コマンドの実行を開始しました: {0} {1}", 
                p.StartInfo.FileName, p.StartInfo.Arguments));
        }

        private void onProcessExited(object sender, EventArgs e)
        {
            // プロセスのリターンコードを取得
            bool ret = (p.ExitCode == 0);

            // 実行結果をログ出力
            outputLogToFile(string.Format(
                "コマンドの実行が{0}しました: {1} {2}",
                ret ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE,
                p.StartInfo.FileName, p.StartInfo.Arguments));

            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.onAppMainProcessExited(ret);
        }

        private static void processOutputDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            standardOutputs.AppendLine(args.Data);
        }

        private static void processErrorDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            standardErrors.AppendLine(args.Data);
        }

        public string getProcessOutputData()
        {
            if (commandAvailable) {
                return standardOutputs.ToString();
            } else {
                return "";
            }
        }

        public string getProcessErrorData()
        {
            if (commandAvailable) {
                return standardErrors.ToString();
            } else {
                return "";
            }
        }

        public bool checkChromeNMSettingFileIsExist() {
            // Chrome Native Messaging設定用のJSONファイルが
            // 導入されているかチェック
            if (File.Exists(ChromeNMSettingFile)) {
                return true;
            }

            outputLogToFile(ChromeNMSettingFile + "が導入されていません");
            return false;
        }

        public void doPairing()
        {
            // U2FMaintenanceTool.exe -Pを実行する
            doCommandWithExecutable(U2FMaintenanceToolExe, "-P");
        }

        public void doEraseBond()
        {
            // U2FMaintenanceTool.exe -Bを実行する
            doCommandWithExecutable(U2FMaintenanceToolExe, "-B");
        }

        public void doEraseSkeyCert()
        {
            // U2FMaintenanceTool.exe -Eを実行する
            doCommandWithExecutable(U2FMaintenanceToolExe, "-E");
        }

        public void doInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // U2FMaintenanceTool.exe -I <skeyFilePath> <certFilePath> を実行する
            string arguments = string.Format("-I {0} {1}", skeyFilePath, certFilePath);
            doCommandWithExecutable(U2FMaintenanceToolExe, arguments);
        }

        public void doHealthCheck()
        {
            // U2FMaintenanceTool.exe -Hを実行する
            doCommandWithExecutable(U2FMaintenanceToolExe, "-H");
        }

        public void doSetupChromeNativeMessaging()
        {
            // U2FMaintenanceTool.exe -R <ChromeNMRegistryKey> <ChromeNMSettingFile> を実行する
            string arguments = string.Format("-R {0} {1}", ChromeNMRegistryKey, ChromeNMSettingFile);
            doCommandWithExecutable(U2FMaintenanceToolExe, arguments);
        }

        public void doCreatePrivateKey(string filePath)
        {
            // openssl.exe ecparam -out <filePath> -name prime256v1 -genkey -noout を実行する
            string arguments = string.Format("ecparam -out {0} -name prime256v1 -genkey -noout", filePath);
            doCommandWithExecutable("openssl.exe", arguments);
        }

        public void doCreateCertReq(string filePath, string certReqParamKeyFile, string certReqParamSubject)
        {
            // openssl.exe req -new -key <certReqParamKeyFile> -subj "<certReqParamSubject>" -out <filePath> を実行する
            string arguments = string.Format("req -new -key {0} -subj \"{1}\" -out {2} -config openssl.cnf", 
                certReqParamKeyFile, certReqParamSubject, filePath);
            doCommandWithExecutable("openssl.exe", arguments);
        }

        public void doCreateSelfCert(string filePath, string selfCertParamKeyFile, string selfCertParamCsrFile, string selfCertParamDays)
        {
            // openssl.exe x509 -in <selfCertParamCsrFile> -days <selfCertParamDays> -req -signkey <selfCertParamKeyFile> -out <filePath> -outform der -extfile cacertV3.ext を実行する
            string arguments = string.Format("x509 -in {0} -days {1} -req -signkey {2} -out {3} -outform der -extfile {4}",
                selfCertParamCsrFile, selfCertParamDays, selfCertParamKeyFile, filePath, OpenSSLCacertV3Ext);
            doCommandWithExecutable("openssl.exe", arguments);
        }

        public void doExit()
        {
            System.Windows.Forms.Application.Exit();
            outputLogToFile("U2F管理ツールを終了しました");
        }
    }
}
