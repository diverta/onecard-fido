using System.Reflection;

namespace ToolAppCommon
{
    public class AppUtil
    {
        private static AppUtil Instance = new AppUtil();
        private string AppVersion;

        private AppUtil()
        {
            // ツールのバージョンを取得
            AppVersion = string.Format("Version {0}", GetAppVersion());
        }

        private static string GetAppVersion()
        {
            // 製品バージョン文字列を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            System.Diagnostics.FileVersionInfo ver = System.Diagnostics.FileVersionInfo.GetVersionInfo(asm.Location);
            string? versionString = ver.ProductVersion;
            if (versionString == null) {
                return "";
            } else {
                return versionString;
            }
        }

        //
        // 公開用メソッド
        //
        public static string GetAppVersionString()
        {
            return Instance.AppVersion;
        }
    }
}
