using System;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    public class PIVImportKeyParameter
    {
        // スロットID
        public byte PkeySlotId { get; set; }

        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }
        public byte PkeyAlgorithm { get; set; }
        public byte CertAlgorithm { get; set; }

        // 秘密鍵インポート用APDU
        public byte[] PkeyAPDUBytes = Array.Empty<byte>();

        // 証明書インポート用APDU
        public byte[] CertAPDUBytes = Array.Empty<byte>();

        // RSA鍵のバイナリーイメージ
        public byte[] RsaEBytes = Array.Empty<byte>();
        public byte[] RsaPBytes = Array.Empty<byte>();
        public byte[] RsaQBytes = Array.Empty<byte>();
        public byte[] RsaDpBytes = Array.Empty<byte>();
        public byte[] RsaDqBytes = Array.Empty<byte>();
        public byte[] RsaQinvBytes = Array.Empty<byte>();

        // EC鍵のバイナリーイメージ
        public byte[] ECPrivKeyBytes = Array.Empty<byte>();

        // 証明書のバイナリーイメージ
        public byte[] CertBytes = Array.Empty<byte>();

        public PIVImportKeyParameter(byte pkeySlotId)
        {
            PkeySlotId = pkeySlotId;
            PkeyAlgName = string.Empty;
            CertAlgName = string.Empty;
        }
    }

    internal class PIVImportKeyProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoProcess(PIVParameter parameterRef, HandlerOnCommandResponse handlerRef)
        {
            // パラメーターを保持
            Parameter = parameterRef;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // 鍵・証明書ファイルを読込み、インポート処理用のリクエストデータを生成
            string errorMessage;
            if (PIVImportKeyRequest.PrepareRequestDataForImport(Parameter, out errorMessage) == false) {
                OnCommandResponse(false, errorMessage);
                return;
            }

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // PIV機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                return;
            }

            // CCID I/F経由で、PIN番号による認証を実行
            new PIVCCIDProcess().DoProcess(Parameter, DoResponsePinAuth);
        }

        private void DoResponsePinAuth(bool success, string errorMessage)
        {
            if (success == false) {
                DoCommandResponse(false, errorMessage);
                return;

            } else {
                // PIN番号による認証が成功
                AppLogUtil.OutputLogInfo(AppCommon.MSG_PIV_PIN_AUTH_SUCCESS);
            }

            // スロット１の秘密鍵をインポート
            DoRequestPivImportKeySlot1();
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }

        //
        // 鍵・証明書インポート
        //
        private delegate void HandlerPivImportKey();
        private delegate void HandlerPivImportCert();

        private void DoRequestPivImportKeySlot1()
        {
            // スロット１の秘密鍵をインポート
            DoRequestPivImportKey(Parameter.ImportKeyParameter1, DoResponsePivImportKeySlot1);
        }

        private void DoResponsePivImportKeySlot1(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、証明書インポート処理に移行
            DoResponsePivImportKey(Parameter.ImportKeyParameter1, success, responseSW, DoRequestPivImportCertSlot1);
        }

        private void DoRequestPivImportCertSlot1()
        {
            // スロット１の証明書をインポート
            DoRequestPivImportCert(Parameter.ImportKeyParameter1, DoResponsePivImportCertSlot1);
        }

        private void DoResponsePivImportCertSlot1(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、次スロットのインポート処理に移行
            DoResponsePivImportCert(Parameter.ImportKeyParameter1, success, responseSW, DoRequestPivImportKeySlot2);
        }

        private void DoRequestPivImportKeySlot2()
        {
            // スロット２の秘密鍵をインポート
            DoRequestPivImportKey(Parameter.ImportKeyParameter2, DoResponsePivImportKeySlot2);
        }

        private void DoResponsePivImportKeySlot2(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、証明書インポート処理に移行
            DoResponsePivImportKey(Parameter.ImportKeyParameter2, success, responseSW, DoRequestPivImportCertSlot2);
        }

        private void DoRequestPivImportCertSlot2()
        {
            // スロット２の証明書をインポート
            DoRequestPivImportCert(Parameter.ImportKeyParameter2, DoResponsePivImportCertSlot2);
        }

        private void DoResponsePivImportCertSlot2(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、次スロットのインポート処理に移行
            DoResponsePivImportCert(Parameter.ImportKeyParameter2, success, responseSW, DoRequestPivImportKeySlot3);
        }

        private void DoRequestPivImportKeySlot3()
        {
            // スロット３の秘密鍵をインポート
            DoRequestPivImportKey(Parameter.ImportKeyParameter3, DoResponsePivImportKeySlot3);
        }

        private void DoResponsePivImportKeySlot3(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、証明書インポート処理に移行
            DoResponsePivImportKey(Parameter.ImportKeyParameter3, success, responseSW, DoRequestPivImportCertSlot3);
        }

        private void DoRequestPivImportCertSlot3()
        {
            // スロット３の証明書をインポート
            DoRequestPivImportCert(Parameter.ImportKeyParameter3, DoResponsePivImportCertSlot3);
        }

        private void DoResponsePivImportCertSlot3(bool success, byte[] responseData, UInt16 responseSW)
        {
            // レスポンスをチェック後、終了処理に移行
            DoResponsePivImportCert(Parameter.ImportKeyParameter3, success, responseSW, DoRequestPivImportKeySlotTerminate);
        }

        private void DoRequestPivImportKeySlotTerminate()
        {
            // 上位クラスに制御を戻す
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // 秘密鍵インポート共通処理
        //
        private static void DoRequestPivImportKey(PIVImportKeyParameter parameter, CCIDProcess.HandlerOnReceivedResponse handler)
        {
            // パラメーターを取得
            byte alg = parameter.PkeyAlgorithm;
            byte slotId = parameter.PkeySlotId;
            byte[] apdu = parameter.PkeyAPDUBytes;

            // コマンドを実行
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.YKPIV_INS_IMPORT_ASYMM_KEY, alg, slotId, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, handler);
        }

        private void DoResponsePivImportKey(PIVImportKeyParameter parameter, bool success, UInt16 responseSW, HandlerPivImportCert? handlerPivImportCert)
        {
            // パラメーターを取得
            byte alg = parameter.PkeyAlgorithm;
            byte slotId = parameter.PkeySlotId;

            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                string errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_IMPORT_PKEY_FAILED, slotId, alg);
                DoCommandResponse(false, errorMessage);
                return;
            }

            // 処理成功のログを出力
            string msgSuccess = string.Format(AppCommon.MSG_PIV_PKEY_PEM_IMPORTED, slotId, alg);
            AppLogUtil.OutputLogInfo(msgSuccess);

            // 証明書インポート処理に移行
            if (handlerPivImportCert != null) {
                handlerPivImportCert();
            }
        }

        //
        // 証明書インポート共通処理
        //
        private void DoRequestPivImportCert(PIVImportKeyParameter parameter, CCIDProcess.HandlerOnReceivedResponse handler)
        {
            // パラメーターを取得
            byte slotId = parameter.PkeySlotId;
            byte[] certApdu = parameter.CertAPDUBytes;

            // コマンドを実行
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_PUT_DATA, 0x3f, 0xff, certApdu, 0xff);
            CCIDProcess.DoRequestCommand(param, handler);
        }

        private void DoResponsePivImportCert(PIVImportKeyParameter parameter, bool success, UInt16 responseSW, HandlerPivImportKey? handlerPivImportKey)
        {
            // パラメーターを取得
            byte alg = parameter.PkeyAlgorithm;
            byte slotId = parameter.PkeySlotId;

            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                string errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_IMPORT_CERT_FAILED, slotId, alg);
                DoCommandResponse(false, errorMessage);
                return;
            }

            // 処理成功のログを出力
            string msgSuccess = string.Format(AppCommon.MSG_PIV_CERT_PEM_IMPORTED, slotId, alg);
            AppLogUtil.OutputLogInfo(msgSuccess);

            // 次スロットの秘密鍵インポート処理が指定されている場合は実行
            if (handlerPivImportKey != null) {
                handlerPivImportKey();
            }
        }
    }
}
