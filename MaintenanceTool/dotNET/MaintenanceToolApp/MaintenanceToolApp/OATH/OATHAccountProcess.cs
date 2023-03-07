using MaintenanceToolApp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ToolAppCommon;

namespace MaintenanceTool.OATH
{
    internal class OATHAccountProcess
    {
        // 処理実行のためのプロパティー
        private readonly OATHParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerNotifyProcessTerminated(bool success, string errorMessage);
        private event HandlerNotifyProcessTerminated NotifyProcessTerminated = null!;

        //
        // 外部公開用
        //
        public OATHAccountProcess(OATHParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoAccountAdd(HandlerNotifyProcessTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyProcessTerminated = handler;

            // ワンタイムパスワード生成処理を実行
            DoRequestAccountAdd();
        }

        public void DoAccountList(HandlerNotifyProcessTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyProcessTerminated = handler;

            // アカウント一覧取得処理を実行
            DoRequestAccountList();
        }

        //
        // アカウント登録処理
        //
        private void DoRequestAccountAdd()
        {
            // APDUを生成
            byte[] apduBytes;
            if (GenerateAccountAddAPDU(Parameter, out apduBytes) == false) {
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OATH_ACCOUNT_ADD_APDU_FAILED);
                return;
            }

            // アカウント登録コマンドを実行
            CCIDParameter param = new CCIDParameter(0x01, 0x00, 0x00, apduBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseAccountAdd);
        }

        private void DoResponseAccountAdd(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                NotifyProcessTerminated(false, string.Format(AppCommon.MSG_ERROR_OATH_ACCOUNT_ADD_FAILED, responseSW));
                return;
            }

            // 処理成功のログを出力
            AppLogUtil.OutputLogInfo(AppCommon.MSG_INFO_OATH_ACCOUNT_ADD_SUCCESS);

            // 上位クラスに制御を戻す
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        public static bool GenerateAccountAddAPDU(OATHParameter Parameter, out byte[] apduBytes)
        {
            apduBytes = Array.Empty<byte>();
            try {
                // アカウント名をバイト配列化
                string accountText = string.Format("{0}:{1}", Parameter.OATHAccountIssuer, Parameter.OATHAccountName);
                byte[] accountBytes = Encoding.ASCII.GetBytes(accountText);

                // Secret（Base32暗号テキスト）をバイト配列化
                byte[] secretBytes;
                Base32Util.Decode(Parameter.OATHBase32Secret, out secretBytes);

                // 変数初期化
                int apduBytesSize = 2 + accountBytes.Length + 4 + secretBytes.Length;
                apduBytes = new byte[apduBytesSize];
                int offset = 0;

                // アカウント
                apduBytes[offset++] = 0x71;
                apduBytes[offset++] = (byte)accountBytes.Length;

                // アカウントをコピー
                Array.Copy(accountBytes, 0, apduBytes, offset, accountBytes.Length);
                offset += accountBytes.Length;

                // Secret
                apduBytes[offset++] = 0x73;
                apduBytes[offset++] = (byte)(secretBytes.Length + 2);
                apduBytes[offset++] = 0x21;
                apduBytes[offset++] = 0x06;

                // Secretをコピー
                Array.Copy(secretBytes, 0, apduBytes, offset, secretBytes.Length);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("OATHProcess.GenerateAccountAddAPDU: {0}", e.Message));
                return false;
            }
        }

        //
        // アカウント一覧取得処理
        //
        private void DoRequestAccountList()
        {
            // APDUを生成
            byte[] apduBytes = Array.Empty<byte>();

            // アカウント登録コマンドを実行
            CCIDParameter param = new CCIDParameter(0x03, 0x00, 0x00, apduBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseAccountList);
        }

        private void DoResponseAccountList(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                NotifyProcessTerminated(false, string.Format(AppCommon.MSG_ERROR_OATH_LIST_ACCOUNT_FAILED, responseSW));
                return;
            }

            // レスポンスからアカウント名一覧を抽出
            Parameter.AccountList.Clear();
            ParseAccountListBytes(responseData, Parameter.AccountList);

            // 上位クラスに制御を戻す
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        private void ParseAccountListBytes(byte[] accountListBytes, List<string> accountList)
        {
            int i = 0;
            while (i < accountListBytes.Length) {
                // 0x71（アカウント名）出現まで走査
                if (accountListBytes[i++] != 0x71) {
                    continue;
                }

                // アカウント名の長さを取得
                int nameLength = accountListBytes[i++];
                if (nameLength == 0 || i > accountListBytes.Length) {
                    continue;
                }

                // アカウント名を抽出し、引数の領域に追加
                byte[] nameBytes = accountListBytes.Skip(i).Take(nameLength).ToArray();
                string nameString = Encoding.UTF8.GetString(nameBytes);
                accountList.Add(nameString);

                // 後続バイトを走査
                i += nameLength;
            }
        }
    }
}
