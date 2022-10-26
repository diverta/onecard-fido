using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using ToolAppCommon;

namespace MaintenanceToolApp.OpenPGP
{
    public class Gpg4winParameter
    {
        public enum GPGCommand
        {
            COMMAND_GPG_NONE = 1,
            COMMAND_GPG_VERSION,
            COMMAND_GPG_MAKE_TEMP_FOLDER,
            COMMAND_GPG_GENERATE_MAIN_KEY,
            COMMAND_GPG_ADD_SUB_KEY,
            COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP,
            COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD,
            COMMAND_GPG_REMOVE_TEMP_FOLDER,
            COMMAND_GPG_CARD_STATUS,
            COMMAND_GPG_CARD_RESET,
            COMMAND_GPG_CARD_EDIT_PASSWD,
            COMMAND_GPG_CARD_EDIT_UNBLOCK,
        };

        // 実行するコマンド
        public GPGCommand Command { get; set; }
        public string CommandPath { get; set; }
        public string CommandArgs { get; set; }
        public string WorkingDirectory { get; set; }

        public Gpg4winParameter(GPGCommand command, string commandPath, string commandArgs, string workingDirectory)
        {
            Command = command;
            CommandPath = commandPath;
            CommandArgs = commandArgs;
            WorkingDirectory = workingDirectory;
        }

        public override string ToString()
        {
            return string.Format("Gpg4winParameter:\nCommand={0}, CommandPath={1} \nCommandArgs:{2} \nWorkingDirectory:{3}",
                Command, CommandPath, CommandArgs, WorkingDirectory);
        }
    }

    internal class Gpg4winProcess
    {
        // 処理実行のためのプロパティー
        private Gpg4winParameter Parameter = null!;

        //
        // GPGコマンドラインプロセッサー
        //
        public delegate void HandlerOnCommandResponse(bool success, string standardOutput, string standardError);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoRequestCommandLine(Gpg4winParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // 実行コマンドに関する諸設定
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = Parameter.CommandPath;
            psi.Arguments = Parameter.CommandArgs;
            psi.UseShellExecute = false;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.StandardOutputEncoding = Encoding.UTF8;
            psi.StandardErrorEncoding = Encoding.UTF8;
            if (Parameter.WorkingDirectory != null) {
                psi.WorkingDirectory = Parameter.WorkingDirectory;
            }
            psi.CreateNoWindow = true;

            // 出力格納領域を初期化
            string stdOutputString = "";
            string stdErrorString = "";
            bool success = false;

            try {
                // コマンドを実行
                Process? child = Process.Start(psi);
                if (child != null) {
                    stdOutputString = child.StandardOutput.ReadToEnd();
                    stdErrorString = child.StandardError.ReadToEnd();

                    // コマンドからの応答を待機
                    child.WaitForExit();

                    // コマンドの戻り値が０であれば true
                    if (child.ExitCode == 0) {
                        success = true;
                    }
                } else {
                    AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.DoRequestCommandLine: Process is null: {0}", parameter.CommandPath));
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.DoRequestCommandLine exception:\n{0}", e.Message));
            }

            // コマンドからの応答文字列／エラー出力を戻す
            OnCommandResponse(success, stdOutputString, stdErrorString);
        }

        //
        // Gpg4winコマンド実行時に使用する
        // 作業用フォルダーの生成／消去処理
        //
        public delegate void HandlerOnTempFolderCommandResponse(bool success, string tempFolderPath);
        private HandlerOnTempFolderCommandResponse OnTempFolderCommandResponse = null!;

        public void MakeTempFolder(HandlerOnTempFolderCommandResponse handlerRef)
        {
            // コールバックを保持
            OnTempFolderCommandResponse = handlerRef;

            string TempFolderPath = string.Empty;
            bool success = false;

            try {
                // 作業用フォルダーを生成
                TempFolderPath = Path.GetTempFileName();
                File.Delete(TempFolderPath);
                Directory.CreateDirectory(TempFolderPath);
                success = true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.MakeTempFolder exception:\n{0}", e.Message));
            }

            // 生成された作業用フォルダーを戻す
            OnTempFolderCommandResponse(success, TempFolderPath);
        }

        public void RemoveTempFolder(string tempFolderPath, HandlerOnTempFolderCommandResponse handlerRef)
        {
            // コールバックを保持
            OnTempFolderCommandResponse = handlerRef;

            bool success = false;

            try {
                // 作業用フォルダーを、内包しているファイルごと削除
                Directory.Delete(tempFolderPath, true);
                success = true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.RemoveTempFolder exception:\n{0}", e.Message));
            }

            // 作業用フォルダー削除の成否を戻す
            OnTempFolderCommandResponse(success, tempFolderPath);
        }

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
            if (Gpg4winUtility.WriteStringToFile(scriptContent, scriptFilePath) == false) {
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
            if (Gpg4winUtility.WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
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
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.GetScriptResourceContent exception:\n{0}", e.Message));
            }

            return ret;
        }
    }
}
