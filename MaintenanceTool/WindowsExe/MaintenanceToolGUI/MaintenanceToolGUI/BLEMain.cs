using System;
using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    internal class BLEMain
    {
        private enum BLERequestType
        {   
            None = 0,
            TestBLEPing,
            TestBLECTAP2,
            TestBLEU2F,
        };
        private BLERequestType bleRequestType = BLERequestType.None;

        // リクエストデータ格納領域
        private byte[] U2FRequestData = new byte[1024];

        // レスポンスデータ格納領域
        private byte[] BLEResponseData = new byte[1024];
        private int BLEResponseLength = 0;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // BLEデバイス関連
        private BLEProcess bleProcess = new BLEProcess();

        // CTAP2、U2F共通処理
        private Ctap2 ctap2;
        private U2f u2f;

        public BLEMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // BLEデバイス関連
            bleProcess.FIDOPeripheralPaired += new BLEProcess.FIDOPeripheralPairedEvent(OnPairedDevice);
            bleProcess.MessageTextEvent += new BLEProcess.MessageTextEventHandler(OnPrintMessageText);
            bleProcess.ReceiveBLEMessageEvent += new BLEProcess.ReceiveBLEMessageEventHandler(OnReceiveBLEMessage);

            AppCommon.OutputLogToFile(String.Format("{0}を起動しました", MainForm.MaintenanceToolTitle), true);

            // CTAP2共通処理に各種参照を引き渡す
            ctap2 = new Ctap2(mainForm, AppCommon.TRANSPORT_BLE);
            ctap2.SetBleMain(this);

            // U2F共通処理に各種参照を引き渡す
            u2f = new U2f(mainForm, AppCommon.TRANSPORT_BLE);
            u2f.SetBleMain(this);
        }

        private void OnPrintMessageText(string message)
        {
            // メッセージを画面表示させる
            mainForm.OnPrintMessageText(message);
        }

        public void doPairing()
        {
            bleProcess.PairWithFIDOPeripheral();
        }

        public void OnPairedDevice(bool success, string messageOnFail)
        {
            if (success == false) {
                // BLEペアリングエラー時は、メッセージを画面表示させる
                mainForm.OnPrintMessageText(messageOnFail);
            }
            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.OnAppMainProcessExited(success);
        }

        private void OnReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            if (ret == false) {
                // BLE接続失敗時等のエラー発生時は画面に制御を戻す
                // 致命的なエラーとなるため、BLE機能のメニューを使用不可にし、
                // アプリケーションを再起動させる必要がある旨のメッセージを表示
                mainForm.OnBLEConnectionDisabled();
                return;
            }

            switch (bleRequestType) {
            case BLERequestType.TestBLEPing:
                DoResponseBLEPing(receivedMessage, receivedLen);
                break;
            case BLERequestType.TestBLECTAP2:
                DoResponseBLECTAP2(receivedMessage, receivedLen);
                break;
            case BLERequestType.TestBLEU2F:
                DoResponseBLEU2F(receivedMessage, receivedLen);
                break;
            default:
                break;
            }
        }

        public void SendBLEMessage(byte cmd, byte[] message, int messageLen)
        {
            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateBLERequestBytes(U2FRequestData, cmd, message, messageLen);

            // BLE処理を実行し、メッセージを転送
            bleProcess.DoXferMessage(U2FRequestData, length);
        }

        private int GenerateBLERequestBytes(byte[] u2fRequestData, byte cmd, byte[] requestBytes, int requestLen)
        {
            // ヘッダーにコマンドをセット
            u2fRequestData[0] = cmd;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(requestLen / 256);
            u2fRequestData[2] = (byte)(requestLen % 256);

            // リクエストデータを配列にセット
            Array.Copy(requestBytes, 0, u2fRequestData, U2f.Const.MSG_HEADER_LEN, requestLen);

            return U2f.Const.MSG_HEADER_LEN + requestLen;
        }

        //
        // BLE PINGコマンド
        //
        public void DoTestBLEPing()
        {
            // PINGリクエスト処理を実行
            bleRequestType = BLERequestType.TestBLEPing;
            ctap2.DoRequestPing();
        }

        private void DoResponseBLEPing(byte[] receivedMessage, int receivedLen)
        {
            // BLEヘッダーを除去し、PINGレスポンス処理を実行
            ExtractResponseData(receivedMessage, receivedLen);
            ctap2.DoResponsePing(BLEResponseData, BLEResponseLength);
        }

        //
        // BLE CTAP2ヘルスチェック
        //
        public void DoCtap2Healthcheck(string pin)
        {
            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンド（getKeyAgreement）を
            //   事前実行する必要あり
            bleRequestType = BLERequestType.TestBLECTAP2;
            ctap2.SetClientPin(pin);
            ctap2.SetRequestType(Ctap2.RequestType.TestMakeCredential);
            ctap2.DoGetKeyAgreement();
        }

        private void DoResponseBLECTAP2(byte[] receivedMessage, int receivedLen)
        {
            // BLEヘッダーを除去し、PINGレスポンス処理を実行
            ExtractResponseData(receivedMessage, receivedLen);
            ctap2.DoResponseCtapHidCbor(BLEResponseData, BLEResponseLength);
        }

        //
        // BLE U2Fヘルスチェック
        //
        public void DoU2FHealthCheck()
        {
            // U2F Register処理を実行
            bleRequestType = BLERequestType.TestBLEU2F;
            u2f.SetRequestType(U2f.RequestType.TestRegister);
            u2f.DoRequestRegister();
        }

        private void DoResponseBLEU2F(byte[] receivedMessage, int receivedLen)
        {
            // BLEヘッダーを除去し、U2Fレスポンス処理を実行
            ExtractResponseData(receivedMessage, receivedLen);
            u2f.DoResponse(BLEResponseData, BLEResponseLength);
        }

        private void ExtractResponseData(byte[] receivedMessage, int receivedLen)
        {
            // CBORバイト配列を抽出
            //   receivedMessage の先頭には、
            //   BLEヘッダー（3バイト）が含まれているので
            //   それをスキップして新しい配列にコピー
            byte cnth = receivedMessage[1];
            byte cntl = receivedMessage[2];
            BLEResponseLength = cnth * 256 + cntl;
            Array.Copy(receivedMessage, 3, BLEResponseData, 0, receivedLen - 3);
        }

        public void DisconnectBLE()
        {
            // 接続ずみの場合はBLEデバイスを切断
            bleProcess.DisconnectBLE();
        }
    }
}
