using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.OpenPGP.Gpg4winParameter;

namespace MaintenanceToolApp.OpenPGP
{
    internal class Gpg4winScriptUtil
    {
        //
        // スクリプト／パラメーターファイル関連
        //
        public static bool WriteScriptToTempFolder(string scriptName, OpenPGPParameter parameter)
        {
            // スクリプトをリソースから読込み
            string scriptContent;
            if (GetScriptResourceContentString(scriptName, out scriptContent) == false) {
                return false;
            }

            // スクリプトファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", parameter.TempFolderPath, scriptName);
            if (WriteStringToFile(scriptContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        public static bool WriteParamForGenerateMainKeyToTempFolder(string scriptName, OpenPGPParameter parameter)
        {
            // パラメーターをリソースから読込み
            string scriptContent;
            if (GetScriptResourceContentString(scriptName, out scriptContent) == false) {
                return false;
            }

            // パラメーターを置き換え
            string parameterContent = string.Format(scriptContent, parameter.RealName, parameter.MailAddress, parameter.Comment);

            // パラメーターファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", parameter.TempFolderPath, scriptName);
            if (WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }


        public static bool WriteParamForCardEditUnblockToTempFolder(string scriptName, OpenPGPParameter parameter)
        {
            // パラメーターをリソースから読込み
            string scriptContent;
            if (GetScriptResourceContentString(scriptName, out scriptContent) == false) {
                return false;
            }

            // パラメーターを置き換え
            string parameterContent;
            switch (parameter.Command) {
            case Command.COMMAND_OPENPGP_UNBLOCK:
                parameterContent = string.Format(scriptContent, parameter.CurrentPin, parameter.NewPin, parameter.NewPin);
                break;
            default:
                parameterContent = string.Format(scriptContent, MenuNoForCardEditPasswdCommand(parameter.Command), parameter.CurrentPin, parameter.NewPin, parameter.NewPin);
                break;
            }

            // パラメーターファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", parameter.TempFolderPath, scriptName);
            if (WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        private static string MenuNoForCardEditPasswdCommand(Command command)
        {
            switch (command) {
            case Command.COMMAND_OPENPGP_CHANGE_PIN:
                return "1";
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
                return "2";
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
                return "3";
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
                return "4";
            default:
                return "Q";
            }
        }

        private static bool GetScriptResourceContentString(string scriptName, out string scriptResourceContentString)
        {
            // 戻り値を初期化
            scriptResourceContentString = string.Empty;

            // スクリプトをリソースから読込み
            string scriptResourceName;
            if (GetScriptResourceName(scriptName, out scriptResourceName) == false) {
                AppLogUtil.OutputLogError(string.Format("Script resource name is null: {0}", scriptName));
                return false;
            }
            string scriptContent;
            if (GetScriptResourceContent(scriptResourceName, out scriptContent) == false) {
                AppLogUtil.OutputLogError(string.Format("Script content is null: {0}", scriptResourceName));
                return false;
            }
            scriptResourceContentString = scriptContent;
            return true;
        }

        private static bool GetScriptResourceName(string scriptName, out string scriptResourceName)
        {
            // 戻り値を初期化
            scriptResourceName = string.Empty;

            // 検索対象のリソース名
            string resourceName = string.Format("MaintenanceToolApp.Resources.{0}", scriptName);

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolApp.Resources.<scriptName>"
                // という名称の場合
                if (resName.Equals(resourceName)) {
                    scriptResourceName = resourceName;
                    return true;
                }
            }
            return false;
        }

        private static bool GetScriptResourceContent(string resourceName, out string scriptResourceContent)
        {
            // 戻り値を初期化
            scriptResourceContent = string.Empty;
            bool ret = false;

            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream? stream = assembly.GetManifestResourceStream(resourceName);
            if (stream == null) {
                return false;
            }

            byte[] ScriptContentBytes;
            int ScriptContentSize;
            try {
                // 配列領域を確保
                int streamLength = (int)stream.Length;
                ScriptContentBytes = new byte[streamLength];

                // リソースファイルを配列に読込
                ScriptContentSize = stream.Read(ScriptContentBytes, 0, streamLength);

                // リソースファイルを閉じる
                stream.Close();

                // 読込んだスクリプト内容を戻す
                byte[] b = ScriptContentBytes.Take(ScriptContentSize).ToArray();
                scriptResourceContent = Encoding.UTF8.GetString(b);
                ret = true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winScriptUtil.GetScriptResourceContent exception:\n{0}", e.Message));
            }

            return ret;
        }

        private static bool WriteStringToFile(string contents, string filePath)
        {
            try {
                File.WriteAllText(filePath, contents);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winScriptUtil.WriteStringToFile exception:\n{0}", e.Message));
                return false;
            }
        }
    }
}
