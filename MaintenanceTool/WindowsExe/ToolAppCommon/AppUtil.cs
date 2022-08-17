using System;
using System.Reflection;

namespace ToolAppCommon
{
    public class AppUtil
    {
        private static AppUtil Instance = new AppUtil();
        private readonly string AppVersion;
        private readonly string AppCopyright;

        private AppUtil()
        {
            // ツールのバージョン、著作権情報を取得
            AppVersion = string.Format("Version {0}", GetAppVersion());
            AppCopyright = GetAppCopyright();
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

        private static string GetAppCopyright()
        {
            // 著作権情報を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            Attribute? attribute = Attribute.GetCustomAttribute(asm, typeof(AssemblyCopyrightAttribute));
            if (attribute == null) {
                return "";
            }
            AssemblyCopyrightAttribute copyright = (AssemblyCopyrightAttribute)attribute;
            return copyright.Copyright;
        }

        //
        // 公開用メソッド
        //
        public static string GetAppVersionString()
        {
            return Instance.AppVersion;
        }

        public static string GetAppCopyrightString()
        {
            return Instance.AppCopyright;
        }
    }
}
