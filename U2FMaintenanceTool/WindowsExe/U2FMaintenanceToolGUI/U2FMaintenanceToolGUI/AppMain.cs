using System;
using System.Diagnostics;
using System.IO;
using U2FMaintenanceToolCommon;

namespace U2FMaintenanceToolGUI
{
    internal class AppMain
    {
        private enum BLERequestType
        {   
            None = 0,
            TestRegister,
            TestAuthenticateCheck,
            TestAuthenticate,
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

        // U2F管理ツールの情報
        public const string U2FMaintenanceToolTitle = "U2F Maintenance Tool";

        // OpenSSLコマンドの情報
        public const string OpenSSLExe = "openssl.exe";
        public const string OpenSSLCacertV3Ext = "cacertV3.ext";
        public bool opensslAvailable;

        // リクエストデータ格納領域
        private byte[] U2FRequestData = new byte[1024];

        // U2Fキーハンドルデータを保持
        // (ヘルスチェック処理で使用)
        private byte[] u2FKeyhandleData = new byte[128];

        // 生成されたランダムなチャレンジ、AppIDを保持
        // (ヘルスチェック処理で使用)
        private byte[] nonce = new byte[Const.U2F_NONCE_SIZE];
        private byte[] appid = new byte[Const.U2F_APPID_SIZE];
        private Random random = new Random();

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
            AppCommon.logFileName = "U2FMaintenanceToolGUI.log";

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
            mainForm.OnPrintMessageText(message);
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
            if (opensslAvailable == false) {
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
                ret ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE,
                p.StartInfo.FileName, p.StartInfo.Arguments));

            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.OnAppMainProcessExited(ret);
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

        public void doPairing()
        {
            bleProcess.PairWithOneCardPeripheral();
        }

        public void OnPairedDevice(bool success)
        {
            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.OnAppMainProcessExited(success);
        }

        private void OnReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            if (ret == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (bleRequestType) {
            case BLERequestType.TestRegister:
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticateCheck);
                break;
            case BLERequestType.TestAuthenticateCheck:
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticate);
                break;
            case BLERequestType.TestAuthenticate:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            default:
                break;
            }
        }

        private void DoRequest(byte[] requestBytes, int requestLen, BLERequestType type)
        {
            // BLE処理を実行し、メッセージを転送
            bleRequestType = type;
            bleProcess.DoXferMessage(requestBytes, requestLen);
        }

        private void DoResponse(bool ret, byte[] receivedMessage, int receivedLen)
        {
            // BLEメッセージが返送されて来たら
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(ret);
        }

        private void GenerateNonceBytes()
        {
            // チャレンジにランダム値を設定
            random.NextBytes(nonce);

            // 生成されたランダム値をダンプ
            string dumpNonce = AppCommon.DumpMessage(nonce, nonce.Length);
            AppCommon.OutputLogToFile(string.Format("doHealthCheck: Challenge\r\n{0}", dumpNonce), true);
        }

        private void GenerateAppIDBytes()
        {
            // AppIDにランダム値を設定
            random.NextBytes(appid);

            // 生成されたランダム値をダンプ
            string dumpAppid = AppCommon.DumpMessage(appid, appid.Length);
            AppCommon.OutputLogToFile(string.Format("doHealthCheck: AppId\r\n{0}", dumpAppid), true);
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
            int length = GenerateU2FRegisterBytes(U2FRequestData);

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, BLERequestType.TestRegister);
        }

        private int GenerateU2FAuthenticateBytes(byte[] u2fRequestData, byte authOption)
        {
            int pos;

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = Const.U2F_INS_AUTHENTICATE;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = authOption;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE + Const.U2F_KEYHANDLE_SIZE + 1;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // キーハンドル長を設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = Const.U2F_KEYHANDLE_SIZE;

            // キーハンドルを設定
            Array.Copy(u2FKeyhandleData, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_KEYHANDLE_SIZE);
            pos += Const.U2F_KEYHANDLE_SIZE;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        private byte getAuthOption(BLERequestType type)
        {
            // 処理区分からオプションを設定
            if (type == BLERequestType.TestAuthenticateCheck) {
                return Const.U2F_AUTH_CHECK_ONLY;
            } else {
                return Const.U2F_AUTH_ENFORCE;
            }
        }

        private void DoTestAuthenticate(bool ret, byte[] receivedMessage, int receivedLen, BLERequestType type)
        {
            // 先行のRegister処理が失敗時は以降の処理を行わない
            if (ret == false) {
                mainForm.OnAppMainProcessExited(ret);
            }

            // Registerレスポンスからキーハンドル
            // (71バイト目から64バイト)を切り出して保持
            Array.Copy(receivedMessage, 70, u2FKeyhandleData, 0, Const.U2F_KEYHANDLE_SIZE);

            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FAuthenticateBytes(U2FRequestData, getAuthOption(type));

            if (type == BLERequestType.TestAuthenticate) {
                // BLE U2Fリクエスト転送の前に、
                // One CardのMAIN SWを押してもらうように促す
                // メッセージを画面表示
                mainForm.OnPrintMessageText("U2F Authenticateを開始します.");
                mainForm.OnPrintMessageText("  ユーザー所在確認が必要となりますので、");
                mainForm.OnPrintMessageText("  One Card上のユーザー所在確認LEDが点滅したら、");
                mainForm.OnPrintMessageText("  MAIN SWを１回押してください.");
            }

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, type);
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
            bleProcess.DisconnectBLE();
            System.Windows.Forms.Application.Exit();
            outputLogToFile("U2F管理ツールを終了しました");
        }
    }
}
