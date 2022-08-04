using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public class ToolPIVConst
    {
        public const byte PIV_INS_SELECT = 0xA4;
        public const byte PIV_INS_VERIFY = 0x20;
        public const byte PIV_INS_GET_DATA = 0xcb;
        public const byte PIV_INS_PUT_DATA = 0xdb;
        public const byte PIV_INS_AUTHENTICATE = 0x87;
        public const byte YKPIV_INS_IMPORT_ASYMM_KEY = 0xfe;
        public const byte PIV_KEY_PIN = 0x80;
        public const byte PIV_KEY_AUTHENTICATION = 0x9a;
        public const byte PIV_KEY_CARDMGM = 0x9b;
        public const byte PIV_KEY_SIGNATURE = 0x9c;
        public const byte PIV_KEY_KEYMGM = 0x9d;
        public const UInt32 PIV_OBJ_CAPABILITY = 0x5fc107;
        public const UInt32 PIV_OBJ_CHUID = 0x5fc102;
        public const UInt32 PIV_OBJ_AUTHENTICATION = 0x5fc105;
        public const UInt32 PIV_OBJ_SIGNATURE = 0x5fc10a;
        public const UInt32 PIV_OBJ_KEY_MANAGEMENT = 0x5fc10b;
        public const byte TAG_DYNAMIC_AUTH_TEMPLATE = 0x7c;
        public const byte TAG_AUTH_WITNESS = 0x80;
        public const byte TAG_AUTH_CHALLENGE = 0x81;
        public const byte TAG_DATA_OBJECT = 0x5c;
        public const byte TAG_DATA_OBJECT_VALUE = 0x53;
        public const byte TAG_CERT = 0x70;
        public const byte TAG_CERT_COMPRESS = 0x71;
        public const byte TAG_CERT_LRC = 0xfe;
        public const string ALG_NAME_RSA2048 = "RSA2048";
        public const string ALG_NAME_ECCP256 = "ECCP256";
        public const byte CRYPTO_ALG_RSA2048 = 0x07;
        public const byte CRYPTO_ALG_ECCP256 = 0x11;
        public const int RSA2048_PQ_SIZE = 128;
        public const int ECCP256_KEY_SIZE = 32;
    }

    public class ToolPIVParameter
    {
        // 鍵作成用パラメーター
        public byte PkeySlotId { get; set; }
        public string PkeyPemPath { get; set; }
        public string CertPemPath { get; set; }
        public string AuthPin { get; set; }
        public string CurrentPin { get; set; }
        public string RenewalPin { get; set; }
        public AppCommon.RequestType SelectedPinCommand { get; set; }
        public string SelectedPinCommandName { get; set; }
        public byte[] ChuidAPDU { get; set; }
        public byte[] CccAPDU { get; set; }
        public byte PkeyAlgorithm { get; set; }
        public byte CertAlgorithm { get; set; }
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }
        public byte[] PkeyAPDU { get; set; }
        public byte[] CertAPDU { get; set; }
    }

    public class ToolPIVSettingItem
    {
        // PIV設定項目を保持
        public byte Retries { get; set; }

        // PIVデータオブジェクトを保持
        private Dictionary<UInt32, byte[]> DataObject = new Dictionary<UInt32, byte[]>();

        public void SetDataObject(UInt32 objectId, byte[] objectData)
        {
            // データを連想配列にコピー
            byte[] b = new byte[objectData.Length];
            Array.Copy(objectData, b, objectData.Length);
            DataObject[objectId] = b;
        }

        public byte[] GetDataObject(UInt32 objectId)
        {
            return DataObject[objectId];
        }
    }

    public class ToolPIV
    {
        // PIV機能設定画面
        private PIVPreferenceForm PreferenceForm;

        // メイン画面の参照を保持
        private MainForm MainFormRef;

        // 処理クラスの参照を保持
        private HIDMain HidMainRef;
        private ToolPIVCcid PIVCcid;
        private ToolPIVPkeyCert toolPIVPkeyCert;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;

        // 処理機能名を保持
        private string NameOfCommand;

        // エラーメッセージテキストを保持
        private string ErrorMessageOfCommand;

        // コマンドが成功したかどうかを保持
        private bool CommandSuccess;

        // ステータス照会情報を保持
        private string StatusInfoString;

        // リクエストパラメーターを保持
        private ToolPIVParameter Parameter = null;

        public ToolPIV(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            MainFormRef = f;

            // HID処理クラスの参照を保持
            HidMainRef = h;

            // PIV機能設定画面を生成
            PreferenceForm = new PIVPreferenceForm(this);

            // CCID処理クラスを生成
            PIVCcid = new ToolPIVCcid();
            PIVCcid.OnCcidCommandTerminated += OnCcidCommandTerminated;
            PIVCcid.OnCcidCommandNotifyErrorMessage += OnCcidCommandNotifyErrorMessage;
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            PreferenceForm.ShowDialog();
        }

        public string GetPIVStatusInfoString()
        {
            return StatusInfoString;
        }

        public bool CheckUSBDeviceDisconnected()
        {
            return MainFormRef.CheckUSBDeviceDisconnected();
        }

        //
        // ファームウェアリセット用関数
        //
        public void DoCommandResetFirmware()
        {
            // HIDインターフェース経由でファームウェアをリセット
            AppCommon.RequestType requestType = AppCommon.RequestType.HidFirmwareReset;
            NotifyProcessStarted(requestType);
            HidMainRef.DoFirmwareReset(requestType, this);
        }

        public void DoResponseResetFirmware(bool success)
        {
            if (success == false) {
                NotifyErrorMessage(AppCommon.MSG_FIRMWARE_RESET_UNSUPP);
            }
            NotifyProcessTerminated(success);
        }

        //
        // PIV機能設定用関数
        // 
        public void DoPIVCommand(AppCommon.RequestType requestType, ToolPIVParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コマンド開始処理
            NotifyProcessStarted(requestType);

            // コマンドを別スレッドで起動
            Task task = Task.Run(() => {
                // 処理機能に応じ、以下の処理に分岐
                RequestType = requestType;
                switch (RequestType) {
                case AppCommon.RequestType.PIVImportKey:
                    DoRequestPIVImportKey();
                    break;
                case AppCommon.RequestType.PIVSetChuId:
                    DoRequestPIVSetChuId();
                    break;
                case AppCommon.RequestType.PIVStatus:
                    DoRequestPIVStatus();
                    break;
                default:
                    // 画面に制御を戻す
                    NotifyProcessTerminated(false);
                    return;
                }
            });

            // 進捗画面を表示
            CommonProcessingForm.OpenForm(PreferenceForm);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestPIVImportKey()
        {
            // 鍵・証明書をファイルから読込
            if (DoProcessImportKey() == false) {
                NotifyProcessTerminated(CommandSuccess);
                return;
            }

            // 鍵・証明書インポート用のAPDUを生成
            if (GenerateImportKeyAPDU(Parameter) == false) {
                NotifyProcessTerminated(CommandSuccess);
                return;
            }

            // 事前にCCID I/F経由で、PIVアプレットをSELECT
            PIVCcid.DoPIVCcidCommand(RequestType, Parameter);
        }

        private void DoResponsePIVImportKey(bool success)
        {
            // 画面に制御を戻す
            CommandSuccess = success;
            NotifyProcessTerminated(CommandSuccess);
        }

        private void DoRequestPIVSetChuId()
        {
            // CHUID／CCCインポート用のAPDUを生成
            Parameter = new ToolPIVParameter();
            GenerateChuidAndCcc(Parameter);

            // 事前にCCID I/F経由で、PIVアプレットをSELECT
            PIVCcid.DoPIVCcidCommand(RequestType, Parameter);
        }

        private void DoResponsePIVSetChuId(bool success)
        {
            // 画面に制御を戻す
            CommandSuccess = success;
            NotifyProcessTerminated(CommandSuccess);
        }

        private void DoRequestPIVStatus()
        {
            // 事前にCCID I/F経由で、PIVアプレットをSELECT
            PIVCcid.DoPIVCcidCommand(RequestType, Parameter);
        }

        private void DoResponsePIVStatus(bool success)
        {
            // 画面出力情報を編集
            if (success) {
                ToolPIVCertDesc pivCertDesc = new ToolPIVCertDesc();
                StatusInfoString = pivCertDesc.EditDescriptionString(PIVCcid.SettingItem, PIVCcid.GetReaderName());
            } else {
                StatusInfoString = "";
            }

            // 画面に制御を戻す
            CommandSuccess = success;
            NotifyProcessTerminated(CommandSuccess);
        }

        //
        // 内部処理
        //
        private bool DoProcessImportKey()
        {
            // 秘密鍵ファイル、証明書ファイルを読込
            toolPIVPkeyCert = new ToolPIVPkeyCert(Parameter);
            if (ReadPrivateKeyPem(Parameter.PkeyPemPath) == false) {
                return false;
            }
            if (ReadCertificatePem(Parameter.CertPemPath) == false) {
                return false;
            }

            // 鍵・証明書のアルゴリズムが異なる場合は、エラーメッセージを表示し処理中止
            if (Parameter.PkeyAlgorithm != Parameter.CertAlgorithm) {
                NotifyErrorMessage(string.Format(AppCommon.MSG_FORMAT_PIV_PKEY_CERT_ALGORITHM,
                    Parameter.PkeyAlgName, Parameter.CertAlgName));
                return false;
            }

            return true;
        }

        private bool ReadPrivateKeyPem(string pkeyPemPath)
        {
            if (toolPIVPkeyCert.LoadPrivateKey(pkeyPemPath) == false) {
                NotifyErrorMessage(AppCommon.MSG_PIV_LOAD_PKEY_FAILED);
                return false;
            }
            AppUtil.OutputLogInfo(AppCommon.MSG_PIV_PKEY_PEM_LOADED);
            return true;
        }

        private bool ReadCertificatePem(string certPemPath)
        {
            if (toolPIVPkeyCert.LoadCertificate(certPemPath) == false) {
                NotifyErrorMessage(AppCommon.MSG_PIV_LOAD_CERT_FAILED);
                return false;
            }
            AppUtil.OutputLogInfo(AppCommon.MSG_PIV_CERT_PEM_LOADED);
            return true;
        }

        private bool GenerateImportKeyAPDU(ToolPIVParameter parameter)
        {
            if (toolPIVPkeyCert.GeneratePrivateKeyAPDU() == false) {
                return false;
            }
            if (toolPIVPkeyCert.GenerateCertificateAPDU(parameter.PkeySlotId) == false) {
                return false;
            }
            return true;
        }

        private void GenerateChuidAndCcc(ToolPIVParameter parameter)
        {
            // CHUID／CCCインポート用のAPDUを生成
            ToolPIVSetId toolPIVSetId = new ToolPIVSetId();
            parameter.ChuidAPDU = toolPIVSetId.GenerateChuidAPDU();
            parameter.CccAPDU = toolPIVSetId.GenerateCccAPDU();
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted(AppCommon.RequestType requestType)
        {
            // コマンド処理結果を初期化
            CommandSuccess = false;

            // 処理機能に応じ、以下の処理に分岐
            RequestType = requestType;
            switch (RequestType) {
            case AppCommon.RequestType.HidFirmwareReset:
                NameOfCommand = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
                break;
            case AppCommon.RequestType.PIVImportKey:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_IMPORT_KEY;
                break;
            case AppCommon.RequestType.PIVChangePin:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_CHANGE_PIN;
                break;
            case AppCommon.RequestType.PIVChangePuk:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_CHANGE_PUK;
                break;
            case AppCommon.RequestType.PIVUnblockPin:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_UNBLOCK_PIN;
                break;
            case AppCommon.RequestType.PIVStatus:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_STATUS;
                break;
            case AppCommon.RequestType.PIVSetChuId:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_SET_CHUID;
                break;
            case AppCommon.RequestType.PIVReset:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_RESET;
                break;
            default:
                NameOfCommand = "";
                break;
            }

            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, NameOfCommand);
            AppUtil.OutputLogInfo(startMsg);
        }

        private void NotifyErrorMessage(string message)
        {
            // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
            AppUtil.OutputLogError(message.Replace("\n", ""));

            // 戻り先画面に表示させるためのエラーメッセージを保持
            ErrorMessageOfCommand = message;
        }

        private void NotifyProcessTerminated(bool success)
        {
            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                NameOfCommand,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppUtil.OutputLogInfo(formatted);
            } else {
                AppUtil.OutputLogError(formatted);
            }

            // 進捗画面を閉じる
            CommonProcessingForm.NotifyTerminate();

            // 画面に制御を戻す
            PreferenceForm.OnCommandProcessTerminated(RequestType, success, ErrorMessageOfCommand);
        }

        //
        // ToolPIVCcidクラスからのコールバック
        //
        private void OnCcidCommandNotifyErrorMessage(string errorMessage)
        {
            NotifyErrorMessage(errorMessage);
        }

        private void OnCcidCommandTerminated(bool success)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVImportKey:
                DoResponsePIVImportKey(success);
                break;
            case AppCommon.RequestType.PIVSetChuId:
                DoResponsePIVSetChuId(success);
                break;
            case AppCommon.RequestType.PIVStatus:
                DoResponsePIVStatus(success);
                break;
            default:
                NotifyProcessTerminated(false);
                break;
            }
        }
    }
}
