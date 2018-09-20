using System;
using System.Diagnostics;
using System.IO;
using U2FHelper;

namespace U2FMaintenanceToolGUI
{
    class AppMain
    {
        private enum BLERequestType
        {   
            None = 0,
            EraseSkeyCert,
            TestRegister,
        };
        private BLERequestType bleRequestType = BLERequestType.None;

        internal static class Const
        {
            public const int MSG_HEADER_LEN = 3;
            public const int U2F_INS_REGISTER = 0x01;
            public const int U2F_INS_AUTHENTICATE = 0x02;
            public const int U2F_INS_VERSION = 0x03;
            public const int U2F_AUTH_ENFORCE = 0x03;
            public const int U2F_AUTH_CHECK_ONLY = 0x07;
            public const int U2F_APPID_SIZE = 32;
            public const int U2F_NONCE_SIZE = 32;
            public const int U2F_KEYHANDLE_SIZE = 64;
        };

        // U2F管理コマンドの情報
        public const string U2FMaintenanceToolTitle = "U2F Maintenance Tool";
        public const string U2FMaintenanceToolExe = "U2FMaintenanceToolCMD.exe";
        public bool commandAvailable;

        // Chrome Native Messaging設定用の
        // レジストリーキー名、JSONファイル名
        public const string ChromeNMRegistryKey = "jp.co.diverta.chrome.helper.ble.u2f";
        public const string ChromeNMSettingFile = "jp.co.diverta.chrome.helper.ble.u2f.json";

        // OpenSSLコマンドの情報
        public const string OpenSSLExe = "openssl.exe";
        public const string OpenSSLCacertV3Ext = "cacertV3.ext";
        public bool opensslAvailable;

        // U2Fキーハンドルデータを保持
        // (ヘルスチェック処理で使用)
        private byte[] u2FKeyhandleData = new byte[128];

        // 生成されたランダムなチャレンジ、AppIDを保持
        // (ヘルスチェック処理で使用)
        private byte[] nonce = new byte[Const.U2F_NONCE_SIZE];
        private byte[] appid = new byte[Const.U2F_APPID_SIZE];
        private Random random = new Random();

        // メッセージテキスト送信用のイベント
        public delegate void printMessageTextEvent(string messageText);
        public event printMessageTextEvent PrintMessageText;

        // 処理完了時のイベント
        public delegate void processExitedEvent(bool success);
        public event processExitedEvent ProcessExited;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行中の外部プロセスを保持
        private Process p;

        // BLEデバイス関連
        private BLEProcess bleProcess = new BLEProcess();

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

            // BLEデバイス関連
            bleProcess.OneCardPeripheralPaired += new BLEProcess.oneCardPeripheralPairedEvent(OnPairedDevice);
            bleProcess.MessageTextEvent += new BLEProcess.MessageTextEventHandler(OnPrintMessageText);
            bleProcess.ReceiveBLEMessageEvent += new BLEProcess.ReceiveBLEMessageEventHandler(OnReceiveBLEMessage);

            outputLogToFile("U2F管理ツールを起動しました");
        }

        private void OnPrintMessageText(string message)
        {
            // メッセージを画面表示させる
            PrintMessageText(message);
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
                "コマンドの実行が{0}しました: {1} {2}",
                ret ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE,
                p.StartInfo.FileName, p.StartInfo.Arguments));

            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            ProcessExited(ret);
        }

        private void processOutputDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            // メイン画面の参照を経由し処理を実行
            mainForm.onAppMainProcessOutputData(args.Data);
        }

        private void processErrorDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Data)) {
                return;
            }
            // メイン画面の参照を経由し処理を実行
            mainForm.onAppMainProcessErrorData(args.Data);
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
            bleProcess.PairWithOneCardPeripheral();
        }

        public void OnPairedDevice(bool success)
        {
            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            ProcessExited(success);
        }

        private void OnReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            if (ret == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                ProcessExited(false);
                return;
            }

            switch (bleRequestType) {
            case BLERequestType.EraseSkeyCert:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            case BLERequestType.TestRegister:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            default:
                break;
            }
        }

        public void doEraseSkeyCert()
        {
            // リクエストデータ（APDU）を編集し request に格納
            // INS=0x42, P1=0x00
            byte[] u2fVersionFrameData = {
                    0x83, 0x00, 0x04,
                    0x00, 0x42, 0x00, 0x00
                };

            // BLE処理を実行し、メッセージを転送
            bleRequestType = BLERequestType.EraseSkeyCert;
            bleProcess.DoXferMessage(u2fVersionFrameData, u2fVersionFrameData.Length);
        }

        private void DoResponse(bool ret, byte[] receivedMessage, int receivedLen)
        {
            // BLEメッセージが返送されて来たら
            // BLEを切断し画面に制御を戻す
            bleProcess.DisconnectBLE();
            ProcessExited(ret);
        }

        public void doInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // U2FMaintenanceTool.exe -I <skeyFilePath> <certFilePath> を実行する
            string arguments = string.Format("-I {0} {1}", skeyFilePath, certFilePath);
            doCommandWithExecutable(U2FMaintenanceToolExe, arguments);
        }

        private void GenerateNonceBytes()
        {
            // チャレンジにランダム値を設定
            random.NextBytes(nonce);

            // 生成されたランダム値をダンプ
            string dumpNonce = U2FHelper.AppCommon.DumpMessage(nonce, nonce.Length);
            U2FHelper.AppCommon.OutputLogToFile(string.Format("doHealthCheck: Challenge\r\n{0}", dumpNonce), true);
        }

        private void GenerateAppIDBytes()
        {
            // AppIDにランダム値を設定
            random.NextBytes(appid);

            // 生成されたランダム値をダンプ
            string dumpAppid = U2FHelper.AppCommon.DumpMessage(appid, appid.Length);
            U2FHelper.AppCommon.OutputLogToFile(string.Format("doHealthCheck: AppId\r\n{0}", dumpAppid), true);
        }

        private int GenerateU2FRegisterBytes(byte[] u2fRequestData)
        {
            int pos;

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = Const.U2F_INS_REGISTER;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        public void doHealthCheck()
        {
            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();
            GenerateAppIDBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            byte[] U2FRequestData = new byte[128];
            int length = GenerateU2FRegisterBytes(U2FRequestData);

            // BLE処理を実行し、メッセージを転送
            bleRequestType = BLERequestType.TestRegister;
            bleProcess.DoXferMessage(U2FRequestData, length);
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
