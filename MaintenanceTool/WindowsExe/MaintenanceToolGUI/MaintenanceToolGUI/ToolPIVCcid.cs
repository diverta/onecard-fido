using System;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class ToolPIVCcid
    {
        // CCID処理クラスの参照を保持
        CCIDProcess Process;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;
        private byte CommandIns;
        private UInt32 ObjectIdToFetch;

        // エラーメッセージを保持
        private string LastErrorMessageWithException = null;

        // リクエストパラメーターを保持
        private ToolPIVParameter Parameter = null;

        // PIV設定情報を保持
        public ToolPIVSettingItem SettingItem = null;

        // PIV管理機能認証（往路）のチャレンジを保持
        private byte[] PivAuthChallenge = null;

        // 乱数製造用
        private Random random = new Random();

        // CCID I/Fからデータ受信時のイベント
        public delegate void CcidCommandTerminatedEvent(bool success);
        public event CcidCommandTerminatedEvent OnCcidCommandTerminated;

        public delegate void CcidCommandNotifyErrorMessageEvent(string errorMessage);
        public event CcidCommandNotifyErrorMessageEvent OnCcidCommandNotifyErrorMessage;

        public ToolPIVCcid()
        {
            // CCID処理クラスを生成
            Process = new CCIDProcess();
            Process.OnDataReceived += OnDataReceived;
        }

        //
        // PIV機能設定用関数
        // 
        public void DoPIVCcidCommand(AppCommon.RequestType requestType, ToolPIVParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            RequestType = requestType;
            Parameter = parameter;

            // CCIDインタフェース経由で認証器に接続
            if (StartCCIDConnection() == false) {
                // 上位クラスに制御を戻す
                OnCcidCommandTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVImportKey:
            case AppCommon.RequestType.PIVSetChuId:
            case AppCommon.RequestType.PIVStatus:
            case AppCommon.RequestType.PIVChangePin:
            case AppCommon.RequestType.PIVChangePuk:
            case AppCommon.RequestType.PIVUnblockPin:
                // 機能実行に先立ち、PIVアプレットをSELECT
                DoRequestPIVInsSelectApplication();
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void OnDataReceived(byte[] responseData, UInt16 responseSW)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (CommandIns) {
            case ToolPIVConst.PIV_INS_SELECT:
                DoResponsePIVInsSelectApplication(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_AUTHENTICATE:
                DoResponsePIVAdminAuth(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_VERIFY:
                DoResponsePIVInsVerify(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_GET_DATA:
                DoResponsePIVInsGetData(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_PUT_DATA:
                DoResponsePIVInsPutData(responseData, responseSW);
                break;
            case ToolPIVConst.YKPIV_INS_IMPORT_ASYMM_KEY:
                DoResponsePivInsImportKey(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_CHANGE_REFERENCE:
            case ToolPIVConst.PIV_INS_RESET_RETRY:
                DoResponsePinManagement(responseData, responseSW);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private bool StartCCIDConnection()
        {
            // CCIDデバイスに接続
            if (Process.Connect()) {
                return true;

            } else {
                // PIV機能を認識できなかった旨のエラーメッセージを設定
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_SELECTING_CARD_FAIL);
                return false;
            }
        }

        private void NotifyCommandTerminated(bool success)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            Process.Disconnect();
            OnCcidCommandTerminated(success);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestPIVInsSelectApplication()
        {
            // PIV appletを選択
            byte[] aidBytes = new byte[] { 0xa0, 0x00, 0x00, 0x03, 0x08 };
            CommandIns = ToolPIVConst.PIV_INS_SELECT;
            Process.SendIns(CommandIns, 0x04, 0x00, aidBytes, 0xff);
        }

        private void DoResponsePIVInsSelectApplication(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                NotifyCommandTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVImportKey:
            case AppCommon.RequestType.PIVSetChuId:
                // PIV管理機能認証（往路）を実行
                DoRequestPivAdminAuth();
                break;
            case AppCommon.RequestType.PIVStatus:
                // PINリトライカウンターを照会
                DoRequestPivInsVerify(null);
                break;
            case AppCommon.RequestType.PIVChangePin:
            case AppCommon.RequestType.PIVChangePuk:
            case AppCommon.RequestType.PIVUnblockPin:
                DoRequestPinManagement();
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void DoRequestPivAdminAuth()
        {
            // PIV管理機能認証（往路）のリクエストデータを生成
            byte[] apdu = { ToolPIVConst.TAG_DYNAMIC_AUTH_TEMPLATE, 2, ToolPIVConst.TAG_AUTH_WITNESS, 0 };
            PivAuthChallenge = null;

            // コマンドを実行
            // 0x03: CRYPTO_ALG_3DES
            CommandIns = ToolPIVConst.PIV_INS_AUTHENTICATE;
            Process.SendIns(CommandIns, 0x03, ToolPIVConst.PIV_KEY_CARDMGM, apdu, 0xff);
        }

        private void DoRequestPivAdminAuthSecond(byte[] responseData)
        {
            // PIV管理機能認証（往路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
            byte[] encrypted = new byte[8];
            Array.Copy(responseData, 4, encrypted, 0, 8);

            // PIV管理パスワードを使用し、受信チャレンジを復号化
            byte[] witness = DecryptPivAdminAuthWitness(encrypted);
            if (witness == null) {
                // エラーが発生時は制御を戻す
                OnCcidCommandNotifyErrorMessage(LastErrorMessageWithException);
                NotifyCommandTerminated(false);
                return;
            }

            // 8バイトのランダムベクターを送信チャレンジに設定
            PivAuthChallenge = new byte[8];
            random.NextBytes(PivAuthChallenge);

            // PIV管理機能認証（復路）のリクエストデータを生成
            byte[] apdu = new byte[22];
            int offset = 0;
            apdu[offset++] = ToolPIVConst.TAG_DYNAMIC_AUTH_TEMPLATE;
            apdu[offset++] = 20;
            // copy witness
            apdu[offset++] = ToolPIVConst.TAG_AUTH_WITNESS;
            apdu[offset++] = (byte)witness.Length;
            Array.Copy(witness, 0, apdu, offset, witness.Length);
            // copy challenge
            offset += witness.Length;
            apdu[offset++] = ToolPIVConst.TAG_AUTH_CHALLENGE;
            apdu[offset++] = (byte)PivAuthChallenge.Length;
            Array.Copy(PivAuthChallenge, 0, apdu, offset, PivAuthChallenge.Length);

            // コマンドを実行
            // 0x03: CRYPTO_ALG_3DES
            CommandIns = ToolPIVConst.PIV_INS_AUTHENTICATE;
            Process.SendIns(CommandIns, 0x03, ToolPIVConst.PIV_KEY_CARDMGM, apdu, 0xff);
        }

        private void DoResponsePIVAdminAuth(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                if (PivAuthChallenge == null) {
                    OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_REQ_FAILED);
                } else {
                    OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_RES_FAILED);
                }
                NotifyCommandTerminated(false);
                return;
            }

            if (PivAuthChallenge == null) {
                // PIV管理機能認証（復路）を実行
                DoRequestPivAdminAuthSecond(responseData);
                return;
            }

            // 送受信チャレンジの内容一致チェック
            // PIV管理機能認証（復路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
            byte[] encrypted = new byte[8];
            Array.Copy(responseData, 4, encrypted, 0, 8);

            // PIV管理パスワードを使用し、受信チャレンジを復号化
            byte[] witness = DecryptPivAdminAuthWitness(encrypted);
            if (witness == null) {
                // エラーが発生時は制御を戻す
                OnCcidCommandNotifyErrorMessage(LastErrorMessageWithException);
                NotifyCommandTerminated(false);
                return;
            }

            // 送信チャレンジと受信チャレンジの内容が異なる場合はPIV管理認証失敗
            if (PivAuthChallenge.SequenceEqual(witness) == false) {
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_CHALLENGE_DIFF);
                NotifyCommandTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVImportKey:
                // PIN番号認証を実行
                DoRequestPivInsVerify(Parameter.AuthPin);
                break;
            case AppCommon.RequestType.PIVSetChuId:
                // CHUID／CCC設定処理を実行
                DoRequestPivSetChuId(responseData, responseSW);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void DoRequestPivInsVerify(string pinCode)
        {
            // コマンドAPDUを生成
            byte[] apdu;
            if (pinCode == null) {
                apdu = new byte[0];
            } else {
                // PINが指定されている場合は、PINを設定
                // ８文字に足りない場合は、足りない部分を0xffで埋める
                apdu = GeneratePinBytes(pinCode);
            }

            // コマンドを実行
            CommandIns = ToolPIVConst.PIV_INS_VERIFY;
            Process.SendIns(CommandIns, 0x00, ToolPIVConst.PIV_KEY_PIN, apdu, 0xff);
        }

        private void DoResponsePIVInsVerify(byte[] responseData, UInt16 responseSW)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVImportKey:
                DoPivImportKeyProcess(responseData, responseSW);
                break;
            case AppCommon.RequestType.PIVStatus:
                // PINリトライカウンターを照会
                DoPivStatusProcessWithPinRetryResponse(responseData, responseSW);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        //
        // 鍵・証明書インポート
        //
        private void DoPivImportKeyProcess(byte[] responseData, UInt16 responseSW)
        {
            // ステータスワードをチェックし、PIN認証の成否を判定
            if (CheckPivInsReplyUsingPinOrPukWithStatus(responseSW, true) == false) {
                // PIN認証が失敗した場合は処理終了
                NotifyCommandTerminated(false);
                return;
            }

            // 秘密鍵インポート処理を実行
            DoRequestPivInsImportKey(Parameter.PkeyAlgorithm, Parameter.PkeySlotId, Parameter.PkeyAPDU);
        }

        private void DoRequestPivInsImportKey(byte alg, byte slotId, byte[] apdu)
        {
            // コマンドを実行
            CommandIns = ToolPIVConst.YKPIV_INS_IMPORT_ASYMM_KEY;
            Process.SendIns(CommandIns, alg, slotId, apdu, 0xff);
        }

        private void DoResponsePivInsImportKey(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                string msgError = string.Format(AppCommon.MSG_ERROR_PIV_IMPORT_PKEY_FAILED, Parameter.PkeySlotId, Parameter.PkeyAlgorithm);
                OnCcidCommandNotifyErrorMessage(msgError);
                NotifyCommandTerminated(false);
                return;
            }

            // 処理成功のログを出力
            string msgSuccess = string.Format(AppCommon.MSG_PIV_PKEY_PEM_IMPORTED, Parameter.PkeySlotId, Parameter.PkeyAlgorithm);
            AppUtil.OutputLogInfo(msgSuccess);

            // 証明書インポート処理を実行
            DoRequestPivImportCert();
        }

        private void DoRequestPivImportCert()
        {
            // スロットIDからオブジェクトIDを取得
            UInt32 objectId = ToolPIVPkeyCert.GetObjectIdFromSlotId(Parameter.PkeySlotId);

            // コマンドを実行
            DoRequestPIVInsPutData(objectId, Parameter.CertAPDU);
        }

        private void DoResponsePivImportCert(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                string msgError = string.Format(AppCommon.MSG_ERROR_PIV_IMPORT_CERT_FAILED, Parameter.PkeySlotId, Parameter.PkeyAlgorithm);
                OnCcidCommandNotifyErrorMessage(msgError);
                NotifyCommandTerminated(false);
                return;
            }

            // 処理成功のログを出力
            string msgSuccess = string.Format(AppCommon.MSG_PIV_CERT_PEM_IMPORTED, Parameter.PkeySlotId, Parameter.PkeyAlgorithm);
            AppUtil.OutputLogInfo(msgSuccess);

            // 処理成功
            NotifyCommandTerminated(true);
        }

        //
        // CHUID／CCC設定
        //
        private void DoRequestPivSetChuId(byte[] responseData, UInt16 responseSW)
        {
            // CHUIDインポート処理を実行
            DoRequestPIVInsPutData(ToolPIVConst.PIV_OBJ_CHUID, Parameter.ChuidAPDU);
        }

        private void DoResponsePivSetChuId(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_IMPORT_CHUID_FAILED);
                NotifyCommandTerminated(false);
                return;
            }

            // 処理成功ログを出力
            AppUtil.OutputLogInfo(AppCommon.MSG_PIV_CHUID_IMPORTED);

            // CCCインポート処理を実行
            DoRequestPIVInsPutData(ToolPIVConst.PIV_OBJ_CAPABILITY, Parameter.CccAPDU);
        }

        private void DoResponsePivSetCCC(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_IMPORT_CCC_FAILED);
                NotifyCommandTerminated(false);
                return;
            }

            // 処理成功ログを出力
            AppUtil.OutputLogInfo(AppCommon.MSG_PIV_CCC_IMPORTED);
            NotifyCommandTerminated(true);
        }

        //
        // PIVステータス照会
        //
        private void DoPivStatusProcessWithPinRetryResponse(byte[] responseData, UInt16 responseSW)
        {
            if ((responseSW >> 8) == 0x63) {
                // PINリトライカウンターを取得
                byte retries = (byte)(responseSW & 0x000f);
                AppUtil.OutputLogInfo(string.Format(AppCommon.MSG_PIV_PIN_RETRY_CNT_GET, retries));

                // PIV設定情報クラスを生成し、リトライカウンターを格納
                if (SettingItem == null) {
                    SettingItem = new ToolPIVSettingItem();
                }
                SettingItem.Retries = retries;

                // PIVオブジェクト（CHUID）を取得
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_CHUID);

            } else {
                // 不明エラーが発生時は処理失敗ログを出力し、制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED);
                NotifyCommandTerminated(false);
            }
        }

        //
        // PIN番号管理
        //
        private void DoRequestPinManagement()
        {
            // INS、P2を設定
            byte[] insP2 = GetPinManagementInsP2(RequestType);
            byte ins = insP2[0];
            byte p2 = insP2[1];

            // コマンドAPDUを生成
            byte[] apdu = GeneratePinManagementAPDU(Parameter.CurrentPin, Parameter.RenewalPin);

            // PIN管理コマンドを実行
            CommandIns = ins;
            Process.SendIns(CommandIns, 0x00, p2, apdu, 0xff);
        }

        private void DoResponsePinManagement(byte[] responseData, UInt16 responseSW)
        {
            // TODO: 仮の実装です。
            NotifyCommandTerminated(true);
        }

        //
        // PIVデータオブジェクト照会
        //
        private void DoRequestPIVInsGetData(UInt32 objectId)
        {
            // 取得対象オブジェクトをAPDUに格納
            ObjectIdToFetch = objectId;
            byte[] apdu = GetPivInsGetApdu(objectId);

            // コマンドを実行
            CommandIns = ToolPIVConst.PIV_INS_GET_DATA;
            Process.SendIns(CommandIns, 0x3f, 0xff, apdu, 0xff);
        }

        private void DoResponsePIVInsGetData(byte[] responseData, UInt16 responseSW)
        {
            if (responseSW != CCIDConst.SW_SUCCESS) {
                // 処理失敗ログを出力（エラーではなく警告扱いとする）
                AppUtil.OutputLogWarn(string.Format(AppCommon.MSG_ERROR_PIV_DATA_OBJECT_GET_FAILED, ObjectIdToFetch));
                // ブランクデータをPIV設定情報クラスに設定
                SettingItem.SetDataObject(ObjectIdToFetch, new byte[0]);

            } else {
                // 処理成功ログを出力
                AppUtil.OutputLogInfo(string.Format(AppCommon.MSG_PIV_DATA_OBJECT_GET, ObjectIdToFetch));
                // 取得したデータをPIV設定情報クラスに設定
                SettingItem.SetDataObject(ObjectIdToFetch, responseData);
            }

            // オブジェクトIDに応じて後続処理分岐
            switch (ObjectIdToFetch) {
            case ToolPIVConst.PIV_OBJ_CHUID:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_CAPABILITY);
                break;
            case ToolPIVConst.PIV_OBJ_CAPABILITY:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_AUTHENTICATION);
                break;
            case ToolPIVConst.PIV_OBJ_AUTHENTICATION:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_SIGNATURE);
                break;
            case ToolPIVConst.PIV_OBJ_SIGNATURE:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_KEY_MANAGEMENT);
                break;
            case ToolPIVConst.PIV_OBJ_KEY_MANAGEMENT:
                NotifyCommandTerminated(true);
                break;
            default:
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private byte[] GetPivInsGetApdu(UInt32 objectId)
        {
            byte[] apdu = new byte[5];
            int offset = 0;

            // 0x5c: TAG_DATA_OBJECT
            apdu[offset++] = 0x5c;

            // オブジェクト長の情報を設定
            apdu[offset++] = 3;
            apdu[offset++] = (byte)((objectId >> 16) & 0x000000ff);
            apdu[offset++] = (byte)((objectId >> 8) & 0x000000ff);
            apdu[offset++] = (byte)(objectId & 0x000000ff);
            return apdu;
        }

        //
        // PIVデータオブジェクト登録
        //
        private void DoRequestPIVInsPutData(UInt32 objectId, byte[] apdu)
        {
            // コマンドを実行
            ObjectIdToFetch = objectId;
            CommandIns = ToolPIVConst.PIV_INS_PUT_DATA;
            Process.SendIns(CommandIns, 0x3f, 0xff, apdu, 0xff);
        }

        private void DoResponsePIVInsPutData(byte[] responseData, UInt16 responseSW)
        {
            // オブジェクトIDに応じて後続処理分岐
            switch (ObjectIdToFetch) {
            case ToolPIVConst.PIV_OBJ_CHUID:
                // CCC設定処理を実行
                DoResponsePivSetChuId(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_OBJ_CAPABILITY:
                DoResponsePivSetCCC(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_OBJ_AUTHENTICATION:
            case ToolPIVConst.PIV_OBJ_SIGNATURE:
            case ToolPIVConst.PIV_OBJ_KEY_MANAGEMENT:
                DoResponsePivImportCert(responseData, responseSW);
                break;
            default:
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        public string GetReaderName()
        {
            return Process.GetReaderName();
        }

        //
        // Triple DES復号化関連
        //
        // デフォルトの3DES鍵
        private byte[] Default3desKey = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
        };

        private byte[] DecryptPivAdminAuthWitness(byte[] encrypted)
        {
            // Triple DESで復号化
            byte[] transformedBytes = null;
            try {
                DES des = new DESCryptoServiceProvider();
                des.Mode = CipherMode.ECB;
                des.Padding = PaddingMode.None;
                byte[] src = new byte[8];
                Array.Copy(encrypted, src, src.Length);

                des.Key = Default3desKey.Take(8).ToArray();
                ICryptoTransform ct1 = des.CreateDecryptor();
                transformedBytes = ct1.TransformFinalBlock(src, 0, src.Length);
                des.Clear();
                Array.Copy(transformedBytes, src, src.Length);

                des.Key = Default3desKey.Skip(8).Take(8).ToArray();
                ICryptoTransform ct2 = des.CreateEncryptor();
                transformedBytes = ct2.TransformFinalBlock(src, 0, src.Length);
                des.Clear();
                Array.Copy(transformedBytes, src, src.Length);

                des.Key = Default3desKey.Skip(16).Take(8).ToArray();
                ICryptoTransform ct3 = des.CreateDecryptor();
                transformedBytes = ct3.TransformFinalBlock(src, 0, src.Length);
                des.Clear();

            } catch (Exception e) {
                // エラーメッセージを保持
                LastErrorMessageWithException = string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, e.Message);
                return null;
            }

            return transformedBytes;
        }

        //
        // PIN認証応答チェック処理
        //
        private bool CheckPivInsReplyUsingPinOrPukWithStatus(UInt16 responseSW, bool isPinAuth)
        {
            // ステータスワードをチェックし、エラーの種類を判定
            int retries = 3;
            bool isPinBlocked = false;
            if ((responseSW >> 8) == 0x63) {
                // リトライカウンターが戻された場合（入力PIN／PUKが不正時）
                retries = responseSW & 0x000f;
                if (retries < 1) {
                    isPinBlocked = true;
                }

            } else if (responseSW == CCIDConst.SW_ERR_AUTH_BLOCKED) {
                // 入力PIN／PUKがすでにブロックされている場合
                isPinBlocked = true;

            } else if (responseSW != CCIDConst.SW_SUCCESS) {
                // 不明なエラーが発生時
                OnCcidCommandNotifyErrorMessage(string.Format(AppCommon.MSG_ERROR_PIV_UNKNOWN, responseSW));
            }
            // PINブロック or リトライカウンターの状態に応じメッセージを編集
            if (isPinBlocked) {
                OnCcidCommandNotifyErrorMessage(isPinAuth ? AppCommon.MSG_ERROR_PIV_PIN_LOCKED : AppCommon.MSG_ERROR_PIV_PUK_LOCKED);

            } else if (retries < 3) {
                string name = isPinAuth ? "PIN" : "PUK";
                string msg = string.Format(AppCommon.MSG_ERROR_PIV_WRONG_PIN, name, name, retries);
                OnCcidCommandNotifyErrorMessage(msg);
            }
            return (responseSW == CCIDConst.SW_SUCCESS);
        }

        //
        // PIN管理コマンド関連
        //
        private byte[] GetPinManagementInsP2(AppCommon.RequestType requestType)
        {
            // INSとP2を配列で戻す
            byte[] insP2 = new byte[2];
            switch (requestType) {
            case AppCommon.RequestType.PIVChangePin:
                insP2[0] = ToolPIVConst.PIV_INS_CHANGE_REFERENCE;
                insP2[1] = ToolPIVConst.PIV_KEY_PIN;
                break;
            case AppCommon.RequestType.PIVChangePuk:
                insP2[0] = ToolPIVConst.PIV_INS_CHANGE_REFERENCE;
                insP2[1] = ToolPIVConst.PIV_KEY_PUK;
                break;
            case AppCommon.RequestType.PIVUnblockPin:
                insP2[0] = ToolPIVConst.PIV_INS_RESET_RETRY;
                insP2[1] = ToolPIVConst.PIV_KEY_PIN;
                break;
            default:
                insP2[0] = 0;
                insP2[1] = 0;
                break;
            }
            return insP2;
        }

        private byte[] GeneratePinManagementAPDU(string currentPin, string renewalPin)
        {
            // 認証用PINコード、更新用PINコードの順で配列にセット
            byte[] apdu = GeneratePinBytes(currentPin);
            apdu = apdu.Concat(GeneratePinBytes(renewalPin)).ToArray();
            return apdu;
        }

        private byte[] GeneratePinBytes(string pinCode)
        {
            // バイト配列に、引数のPINを設定
            // ８文字に足りない場合は、足りない部分を0xffで埋める
            byte[] apdu = new byte[8];
            byte[] pinCodeBytes = Encoding.ASCII.GetBytes(pinCode);
            for (int i = 0; i < apdu.Length; i++) {
                if (i < pinCodeBytes.Length) {
                    apdu[i] = pinCodeBytes[i];
                } else {
                    apdu[i] = 0xff;
                }
            }
            return apdu;
        }
    }
}
