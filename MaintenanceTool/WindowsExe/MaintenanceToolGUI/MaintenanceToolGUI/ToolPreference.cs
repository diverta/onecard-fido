using MaintenanceToolCommon;
using System.Linq;

namespace MaintenanceToolGUI
{
    public class ToolPreferenceParameter
    {
        // 自動認証設定関連のパラメーター
        public bool BleScanAuthEnabled { get; set; }
        public bool BlePairingIsNeeded { get; set; }
        public string ServiceUUIDString { get; set; }
        public string ServiceUUIDScanSec { get; set; }

        // 自動認証設定コマンドの種別
        public ToolPreference.CommandType CommandType { get; set; }
    }

    public class ToolPreference
    {
        // 画面の参照を保持
        private MainForm mainForm;
        private ToolPreferenceForm toolPreferenceForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;

        // 実行する自動認証設定コマンドの種別
        public enum CommandType
        {
            None = 0,
            COMMAND_AUTH_PARAM_GET,
            COMMAND_AUTH_PARAM_SET,
            COMMAND_AUTH_PARAM_RESET,
            COMMAND_AUTH_PARAM_INQUIRY
        };

        // リクエストパラメーターを保持
        private ToolPreferenceParameter toolPreferenceParameter = null;

        public ToolPreference(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;

            // ツール設定画面を生成
            toolPreferenceForm = new ToolPreferenceForm();
            toolPreferenceForm.SetToolPreferenceRef(this);
        }

        public void SetTitleAndVersionText()
        {
            // ツール名、バージョンを引き渡し
            toolPreferenceForm.SetTitleAndVersionText(
                MainForm.MaintenanceToolTitle, 
                MainForm.MaintenanceToolVersion,
                MainForm.MaintenanceToolCopyright);
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            toolPreferenceForm.ShowDialog();
        }

        public void DoCommandToolPreference(ToolPreferenceParameter parameter)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 処理開始ログ出力
            toolPreferenceForm.OnToolPreferenceCommandStarted();

            // 画面から引き渡されたパラメーターを退避
            toolPreferenceParameter = parameter;

            // HID INITコマンドを実行
            hidMain.DoRequestCtapHidInitByToolPreference(this);
        }

        public void DoToolPreferenceParamInquiry()
        {
            // パラメーターに処理区分のみ設定
            toolPreferenceParameter = new ToolPreferenceParameter();
            toolPreferenceParameter.CommandType = CommandType.COMMAND_AUTH_PARAM_INQUIRY;

            // HID INITコマンドを実行
            hidMain.DoRequestCtapHidInitByToolPreference(this);
        }

        // 
        // INITコマンドの後続処理判定
        //
        public bool DoResponseHidInit(byte[] message, int length)
        {
            switch (toolPreferenceParameter.CommandType) {
            case CommandType.COMMAND_AUTH_PARAM_GET:
            case CommandType.COMMAND_AUTH_PARAM_SET:
            case CommandType.COMMAND_AUTH_PARAM_RESET:
                DoRequestToolPreference();
                break;
            case CommandType.COMMAND_AUTH_PARAM_INQUIRY:
                DoRequestToolPreferenceParamInquiry();
                break;
            default:
                return false;
            }
            return true;
        }

        //
        // 自動認証設定コマンドのリクエスト処理
        //
        public void DoRequestToolPreference()
        {
            // コマンド（1 から始まる値です）
            byte cmd = (byte)toolPreferenceParameter.CommandType;

            // 自動認証フラグを編集
            int pairNeed = toolPreferenceParameter.BlePairingIsNeeded ? 256 : 0;
            int scanEnabled = toolPreferenceParameter.BleScanAuthEnabled ? 1 : 0;
            int flags = pairNeed + scanEnabled;

            // パラメーターからCSVを生成
            string csv = string.Format("{0},{1},{2}",
                flags,
                toolPreferenceParameter.ServiceUUIDString,
                toolPreferenceParameter.ServiceUUIDScanSec);

            // リクエストデータを生成
            byte[] requestData = new byte[] { cmd };
            if (toolPreferenceParameter.CommandType == CommandType.COMMAND_AUTH_PARAM_SET) {
                byte[] csvData = System.Text.Encoding.ASCII.GetBytes(csv);
                requestData = requestData.Concat(csvData).ToArray();
            }

            // HID経由でリクエストデータを送信
            hidMain.SendHIDMessage(Const.HID_CMD_TOOL_PREF_PARAM, requestData, requestData.Length);
        }

        public void DoRequestToolPreferenceParamInquiry()
        {
            // リクエストデータを生成
            byte cmd = (byte)CommandType.COMMAND_AUTH_PARAM_GET;
            byte[] requestData = new byte[] { cmd };

            // HID経由でリクエストデータを送信
            hidMain.SendHIDMessage(Const.HID_CMD_TOOL_PREF_PARAM, requestData, requestData.Length);
        }

        //
        // 自動認証設定コマンドのレスポンス処理
        //
        public void DoResponseToolPreference(byte[] message, int length)
        {
            // ステータスをチェックし 0x00 以外ならエラーとする
            if (message[0] != 0x00) {
                OnHidMainProcessExited(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // ステータスバイトを除いた残りのデータがCSVデータ
            byte[] csvData = AppCommon.ExtractCBORBytesFromResponse(message, length);

            // CSVデータをASCII文字列に変換
            string csv = System.Text.Encoding.ASCII.GetString(csvData);

            // CSVデータを分解し、自動認証パラメーターを抽出
            ToolPreferenceParameter param = ExtractToolPreferenceParameter(csv.Split(','));

            // 自動認証設定照会の場合、
            if (toolPreferenceParameter.CommandType == CommandType.COMMAND_AUTH_PARAM_INQUIRY) {
                // 共有情報にデータをセット
                ToolContext.GetInstance().SetBleScanAuthParamValues(param);
                // メイン画面に制御を戻す
                mainForm.DoResponseToolPreferenceParamInquiry();
                return;
            }

            // CSVデータを分解して画面項目に設定
            toolPreferenceForm.SetFields(param);

            // 処理結果を画面表示し、ボタンを押下可能とする
            OnHidMainProcessExited(true, "");
        }

        private ToolPreferenceParameter ExtractToolPreferenceParameter(string[] fields)
        {
            ToolPreferenceParameter param = new ToolPreferenceParameter();
            if (fields.Length != 3) {
                return param;
            }

            // 配列の先頭から画面項目に設定
            //   自動認証フラグ
            int flags = 0;
            try {
                flags = int.Parse(fields[0]);
            } catch {
            }
            int _iBleScanAuthEnabled = flags % 256;
            int _iBlePairingIsNeeded = flags / 256;
            param.BleScanAuthEnabled = (_iBleScanAuthEnabled == 1);
            param.BlePairingIsNeeded = (_iBlePairingIsNeeded == 1);
            //   スキャン対象UUID
            param.ServiceUUIDString = fields[1];
            //   スキャン秒数
            param.ServiceUUIDScanSec = fields[2];

            return param;
        }

        public void OnHidMainProcessExited(bool ret, string errMessage)
        {
            // コマンドタイムアウト監視終了
            // TODO

            // 処理結果を画面表示し、ボタンを押下可能とする
            toolPreferenceForm.OnToolPreferenceCommandExecuted(ret, errMessage);
        }
    }
}
