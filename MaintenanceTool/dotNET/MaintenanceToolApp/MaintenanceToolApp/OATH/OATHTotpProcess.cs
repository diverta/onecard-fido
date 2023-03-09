using MaintenanceToolApp;
using System;
using System.Text;
using ToolAppCommon;

namespace MaintenanceTool.OATH
{
    internal class OATHTotpProcess
    {
        // 処理実行のためのプロパティー
        private readonly OATHParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerNotifyProcessTerminated(bool success, string errorMessage);
        private event HandlerNotifyProcessTerminated NotifyProcessTerminated = null!;

        //
        // 外部公開用
        //
        public OATHTotpProcess(OATHParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoCalculate(HandlerNotifyProcessTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyProcessTerminated = handler;

            // ワンタイムパスワード生成処理を実行
            DoRequestCalculate();
        }

        //
        // ワンタイムパスワード生成処理
        //
        private void DoRequestCalculate()
        {
            // APDUを生成
            byte[] apduBytes;
            if (GenerateAPDUForCalculate(Parameter, out apduBytes) == false) {
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OATH_CALCULATE_APDU_FAILED);
                return;
            }

            // アカウント登録コマンドを実行
            CCIDParameter param = new CCIDParameter(0x04, 0x00, 0x00, apduBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseCalculate);
        }

        private void DoResponseCalculate(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                NotifyProcessTerminated(false, string.Format(AppCommon.MSG_ERROR_OATH_CALCULATE_FAILED, responseSW));
                return;
            }

            // レスポンスの4～7バイト目をエンディアン変換し、
            // ワンタイムパスワードを生成（下６桁を抽出）
            UInt32 TOTPSrcInt = AppUtil.ToUInt32(responseData, 3, true);
            Parameter.OATHTOTPValue = TOTPSrcInt % 1000000;

            // 処理成功のログを出力
            AppLogUtil.OutputLogInfo(AppCommon.MSG_INFO_OATH_CALCULATE_SUCCESS);

            // 上位クラスに制御を戻す
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        public static bool GenerateAPDUForCalculate(OATHParameter Parameter, out byte[] apduBytes)
        {
            apduBytes = Array.Empty<byte>();
            try {
                // アカウント名をバイト配列化
                string accountText = string.Format("{0}:{1}", Parameter.OATHAccountIssuer, Parameter.OATHAccountName);
                byte[] accountBytes = Encoding.ASCII.GetBytes(accountText);

                // 現在のUNIX時刻を取得
                TimeSpan t = DateTime.UtcNow - new DateTime(1970, 1, 1);
                UInt32 nowEpochSeconds = (UInt32)t.TotalSeconds;
                UInt32 challenge = nowEpochSeconds / 30;

                // Challenge（Unix時間）をビッグエンディアンでバイト配列化
                byte[] challengeBytes = new byte[8];
                AppUtil.ConvertUint32ToBEBytes(challenge, challengeBytes, 4);

                // 変数初期化
                int apduBytesSize = 2 + accountBytes.Length + 2 + challengeBytes.Length;
                apduBytes = new byte[apduBytesSize];
                int offset = 0;

                // アカウント
                apduBytes[offset++] = 0x71;
                apduBytes[offset++] = (byte)accountBytes.Length;

                // アカウントをコピー
                Array.Copy(accountBytes, 0, apduBytes, offset, accountBytes.Length);
                offset += accountBytes.Length;

                // Challenge
                apduBytes[offset++] = 0x74;
                apduBytes[offset++] = (byte)challengeBytes.Length;

                // Challengeをコピー
                Array.Copy(challengeBytes, 0, apduBytes, offset, challengeBytes.Length);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("OATHProcess.GenerateAPDUForCalculate: {0}", e.Message));
                return false;
            }
        }
    }
}
