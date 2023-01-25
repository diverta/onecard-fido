using System;
using System.IO;
using System.Reflection;
using System.Text;

namespace ToolAppCommon
{
    public class AppLogUtil
    {
        private static AppLogUtil Instance = new AppLogUtil();
        private string ApplicationName = "";

        private AppLogUtil()
        {
        }

        //
        // 公開用メソッド
        //
        public static void SetApplicationName(string applicationName)
        {
            Instance.ApplicationName = applicationName;
        }

        public static string GetApplicationName()
        {
            return Instance.ApplicationName;
        }

        public static void OutputLogText(string logText, string fname)
        {
            try {
                // ログファイルにメッセージを出力する
                StreamWriter sr = new StreamWriter(new FileStream(fname, FileMode.Append), Encoding.Default);
                sr.WriteLine(logText);
                sr.Close();

            } catch (Exception e) {
                Console.Write(e.Message);
            }
        }

        public static void SetOutputLogApplName()
        {
            // ログ出力を行うアプリケーション名を設定
            string applName = "ToolApp";
            AssemblyName assemblyName = Assembly.GetExecutingAssembly().GetName();
            if (assemblyName.Name != null) {
                applName = assemblyName.Name;
            }
            SetApplicationName(applName);
        }

        public static void OutputLogText(string logText)
        {
            // アプリケーション名が未設定の場合はログを出力させない
            if (GetApplicationName().Length == 0) {
                return;
            }

            // ログファイルにメッセージを出力する
            string fname = string.Format("{0}\\{1}.log", OutputLogFileDirectoryPath(), GetApplicationName());
            OutputLogText(logText, fname);
        }

        public static string OutputLogFileDirectoryPath()
        {
            try {
                // ホームディレクトリー配下に生成
                string dir = string.Format("{0}\\Diverta\\FIDO",
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));

                // ディレクトリー存在チェック
                if (Directory.Exists(dir) == false) {
                    // ディレクトリーが存在しない場合は新規生成
                    DirectoryInfo dirInfo = Directory.CreateDirectory(dir);
                    Console.Write(string.Format("outputLogText: Directory created at {0}", dir));
                }

                // ディレクトリーを戻す
                return dir;

            } catch (Exception e) {
                Console.Write(e.Message);
                return ".";
            }
        }

        public static void OutputLogError(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [error] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogWarn(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [warn] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogInfo(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [info] {1}", DateTime.Now.ToString(), message));
        }

        public static void OutputLogDebug(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力
            OutputLogText(string.Format("{0} [debug] {1}", DateTime.Now.ToString(), message));
        }

        public static string DumpMessage(byte[] message, int length)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < length; i++) {
                sb.Append(string.Format("{0:x2} ", message[i]));
                if ((i % 16 == 15) && (i < length - 1)) {
                    sb.Append("\r\n");
                }
            }
            return sb.ToString();
        }
    }
}
