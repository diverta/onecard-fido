using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using U2FHelper;

namespace U2FMaintenanceToolGUI
{
    class AppMain
    {
        private enum BLERequestType
        {   
            None = 0,
            EraseSkeyCert,
            InstallSkey,
            InstallCert,
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

        // インストールする証明書ファイルパスを保持
        private string CertFilePath;

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
            case BLERequestType.EraseSkeyCert:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            case BLERequestType.InstallSkey:
                DoInstallCertFile();
                break;
            case BLERequestType.InstallCert:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
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

        public void doEraseSkeyCert()
        {
            // リクエストデータ（APDU）を編集し request に格納
            // INS=0x42, P1=0x00
            byte[] u2fVersionFrameData = {
                    0x83, 0x00, 0x04,
                    0x00, 0x42, 0x00, 0x00
                };

            // BLE処理を実行し、メッセージを転送
            DoRequest(u2fVersionFrameData, u2fVersionFrameData.Length, BLERequestType.EraseSkeyCert);
        }

        private string ReadTextFile(string skeyFilePath)
        {
            string text = "";
            try {
                string line;
                using (StreamReader sr = new StreamReader(
                    skeyFilePath, Encoding.GetEncoding("Shift_JIS"))) {
                    while ((line = sr.ReadLine()) != null) {
                        // ヘッダー／フッターは読み飛ばす
                        if (line.Equals("-----BEGIN EC PRIVATE KEY-----")) {
                            continue;
                        }
                        if (line.Equals("-----END EC PRIVATE KEY-----")) {
                            continue;
                        }
                        text += line;
                    }
                }
            } catch (Exception e) {
                U2FHelper.AppCommon.OutputLogToFile(string.Format("ReadLine failed: {0}", e.Message), true);
            }
            return text;
        }

        private byte[] DecodeB64EncodedString(string strMessage)
        {
            try {
                // non web-safe形式に変換
                string encodedText = strMessage;
                encodedText = encodedText.Replace('_', '/');
                encodedText = encodedText.Replace('-', '+');

                // メッセージをbase64デコード
                byte[] transferMessage = Convert.FromBase64String(encodedText);
                return transferMessage;

            } catch (Exception e) {
                U2FHelper.AppCommon.OutputLogToFile(string.Format("Convert.FromBase64String failed: {0}", e.Message), true);
            }
            return null;
        }

        private bool ReadPemFile(string skeyFilePath, byte[] skeyBytes)
        {
            string pemText = ReadTextFile(skeyFilePath);
            if (pemText.Length == 0) {
                return false;
            }

            byte[] pemBytes = DecodeB64EncodedString(pemText);
            if (pemBytes == null) {
                return false;
            }

            // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
            // 先頭からリトルエンディアン形式で配置しなおす。
            for (int i = 0; i < 32; i++) {
                skeyBytes[31 - i] = pemBytes[7 + i];
            }

            return true;
        }

        private int GenerateInstallSkeyBytes(byte[] u2fRequestData, byte[] skeyBytes, int skeyBytesLen)
        {
            int pos;

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            //   INS=0x43, P1=0x00
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = 0x43;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            //   データ長を設定
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = (byte)(skeyBytesLen / 256);
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = (byte)(skeyBytesLen % 256);
            //   データを設定
            pos = 7;
            Array.Copy(skeyBytes, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, skeyBytesLen);
            pos += skeyBytesLen;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        public void doInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // 証明書ファイルパスを保持
            CertFilePath = certFilePath;

            // 秘密鍵ファイルを読込む
            //   skey_bufferの先頭から32バイト分、
            //   リトルエンディアン形式で格納される
            byte[] skeyBytes = new byte[32];
            if (ReadPemFile(skeyFilePath, skeyBytes) == false) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            byte[] U2FRequestData = new byte[64];
            int length = GenerateInstallSkeyBytes(U2FRequestData, skeyBytes, skeyBytes.Length);

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, BLERequestType.InstallSkey);
        }

        private byte[] ReadCertFile(string certFilePath)
        {
            try {
                byte[] certBytes = File.ReadAllBytes(certFilePath);
                return certBytes;

            } catch (Exception e) {
                U2FHelper.AppCommon.OutputLogToFile(string.Format("File.ReadAllBytes failed: {0}", e.Message), true);
            }

            return null;
        }

        private int GenerateInstallCertBytes(byte[] u2fRequestData, byte[] certBytes, int certBytesLen)
        {
            int pos;

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            //   INS=0x44, P1=0x00
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = 0x44;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            //   データ長を設定
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = (byte)(certBytesLen / 256);
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = (byte)(certBytesLen % 256);
            //   データを設定
            pos = 7;
            Array.Copy(certBytes, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, certBytesLen);
            pos += certBytesLen;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        public void DoInstallCertFile()
        {
            // 証明書ファイルを読込む
            byte[] certBytes = ReadCertFile(CertFilePath);
            if (certBytes == null) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateInstallCertBytes(U2FRequestData, certBytes, certBytes.Length);

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, BLERequestType.InstallCert);
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
            bleProcess.DisconnectBLE();
            System.Windows.Forms.Application.Exit();
            outputLogToFile("U2F管理ツールを終了しました");
        }
    }
}
