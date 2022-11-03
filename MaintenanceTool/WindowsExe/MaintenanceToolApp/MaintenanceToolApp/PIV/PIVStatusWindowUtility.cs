namespace MaintenanceToolApp.PIV
{
    internal class PIVStatusWindowUtility
    {
        public static string EditDescriptionString(PIVParameter parameter, string readerName)
        {
            // パラメーターから設定値を取得
            PIVSettingDataObjects settings = parameter.PIVSettings;
            byte retries = parameter.Retries;

            // 変数を初期化
            string StatusInfoString = "";
            string CRLF = "\r\n";

            // 画面表示される文字列を編集
            StatusInfoString += string.Format("Device: {0}", readerName) + CRLF + CRLF;
            StatusInfoString += string.Format("CHUID:  {0}", PrintableCHUIDString(settings)) + CRLF;
            StatusInfoString += string.Format("CCC:    {0}", PrintableCCCString(settings)) + CRLF + CRLF;
            StatusInfoString += string.Format("PIN tries left: {0}", retries);
            return StatusInfoString;
        }

        private static string PrintableCHUIDString(PIVSettingDataObjects settings)
        {
            byte[] chuid = settings.Get(PIVConst.PIV_OBJ_CHUID);
            return PrintableObjectStringWithData(chuid);
        }

        private static string PrintableCCCString(PIVSettingDataObjects settings)
        {
            byte[] ccc = settings.Get(PIVConst.PIV_OBJ_CAPABILITY);
            return PrintableObjectStringWithData(ccc);
        }

        private static string PrintableObjectStringWithData(byte[] data)
        {
            // ブランクデータの場合
            if (data == null || data.Length == 0) {
                return "No data available";
            }

            // オブジェクトの先頭２バイト（＝TLVタグ）は不要なので削除
            int offset = 2;
            int size = data.Length - offset;

            // データオブジェクトを、表示可能なHEX文字列に変換
            string hex = "";
            for (int i = 0; i < size; i++) {
                hex += string.Format("{0:x2}", data[i + offset]);
            }
            return hex;
        }
    }
}
