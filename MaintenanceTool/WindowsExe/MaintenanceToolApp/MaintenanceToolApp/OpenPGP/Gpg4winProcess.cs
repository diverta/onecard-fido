using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
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
        private event HandlerOnCommandResponse OnCommandResponse = null!;
        private HandlerOnCommandResponse OnCommandResponseRef = null!;

        public void DoRequestCommandLine(Gpg4winParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // イベントを登録
            OnCommandResponseRef = new HandlerOnCommandResponse(handlerRef);
            OnCommandResponse += OnCommandResponseRef;

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

        public void UnregisterHandlerOnCommandResponse()
        {
            // イベントを解除
            OnCommandResponse -= OnCommandResponseRef;
        }

        //
        // Gpg4winコマンド実行時に使用する
        // 作業用フォルダーの生成／消去処理
        //
        public delegate void HandlerOnTempFolderCommandResponse(bool success, string tempFolderPath);
        private event HandlerOnTempFolderCommandResponse OnTempFolderCommandResponse = null!;
        private HandlerOnTempFolderCommandResponse OnTempFolderCommandResponseRef = null!;

        private string TempFolderPath = string.Empty;

        public void MakeTempFolder(HandlerOnTempFolderCommandResponse handlerRef)
        {
            // イベントを登録
            OnTempFolderCommandResponseRef = new HandlerOnTempFolderCommandResponse(handlerRef);
            OnTempFolderCommandResponse += OnTempFolderCommandResponseRef;

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

        public void RemoveTempFolder(HandlerOnTempFolderCommandResponse handlerRef)
        {
            // イベントを登録
            OnTempFolderCommandResponseRef = new HandlerOnTempFolderCommandResponse(handlerRef);
            OnTempFolderCommandResponse += OnTempFolderCommandResponseRef;

            bool success = false;

            try {
                // 作業用フォルダーを、内包しているファイルごと削除
                Directory.Delete(TempFolderPath, true);
                success = true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.RemoveTempFolder exception:\n{0}", e.Message));
            }

            // 作業用フォルダー削除の成否を戻す
            OnTempFolderCommandResponse(success, TempFolderPath);
            TempFolderPath = string.Empty;
        }

        public void UnregisterHandlerOnTempFolderCommandResponse()
        {
            // イベントを解除
            OnTempFolderCommandResponse -= OnTempFolderCommandResponseRef;
        }

        //
        // スクリプト／パラメーターファイル関連
        //
        public bool WriteScriptToTempFolder(string scriptName)
        {
            // スクリプトをリソースから読込み
            string scriptContent = GetScriptResourceContentString(scriptName);
            if (scriptContent == null) {
                return false;
            }

            // スクリプトファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            if (WriteStringToFile(scriptContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        public bool WriteParamForGenerateMainKeyToTempFolder(string scriptName, OpenPGPParameter parameter)
        {
            // パラメーターをリソースから読込み
            string scriptContent = GetScriptResourceContentString(scriptName);
            if (scriptContent == string.Empty) {
                return false;
            }

            // パラメーターを置き換え
            string parameterContent = string.Format(scriptContent, parameter.RealName, parameter.MailAddress, parameter.Comment);

            // パラメーターファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            if (WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        private string GetScriptResourceContentString(string scriptName)
        {
            // スクリプトをリソースから読込み
            string scriptResourceName = GetScriptResourceName(scriptName);
            if (scriptResourceName == string.Empty) {
                AppLogUtil.OutputLogError(string.Format("Script resource name is null: {0}", scriptName));
                return string.Empty;
            }
            string scriptContent = GetScriptResourceContent(scriptResourceName);
            if (scriptContent == null) {
                AppLogUtil.OutputLogError(string.Format("Script content is null: {0}", scriptResourceName));
                return string.Empty;
            }
            return scriptContent;
        }

        private static string GetScriptResourceName(string scriptName)
        {
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
                    return resourceName;
                }
            }
            return string.Empty;
        }

        // スクリプト内容を読込むための領域
        private byte[] ScriptContentBytes = new byte[5120];
        private int ScriptContentSize { get; set; }

        private string GetScriptResourceContent(string resourceName)
        {
            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream? stream = assembly.GetManifestResourceStream(resourceName);
            if (stream == null) {
                return string.Empty;
            }

            try {
                // リソースファイルを配列に読込
                ScriptContentSize = stream.Read(ScriptContentBytes, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.GetScriptResourceContent exception:\n{0}", e.Message));
                return string.Empty;
            }

            // 読込んだスクリプト内容を戻す
            byte[] b = ScriptContentBytes.Take(ScriptContentSize).ToArray();
            string text = Encoding.UTF8.GetString(b);
            return text;
        }

        //
        // ユーティリティー
        //
        public static bool CheckResponseOfScript(string response)
        {
            // メッセージ検索用文字列
            string keyword = "Execute script for gnupg success";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keyword)) {
                    // シェルスクリプトから成功メッセージが出力された場合、trueを戻す
                    return true;
                }
            }
            return false;
        }

        public static bool CheckIfGPGVersionAvailable(string response)
        {
            // メッセージ検索用文字列
            string keyword = "gpg (GnuPG) ";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.StartsWith(keyword)) {
                    // バージョン文字列を抽出
                    string versionStr = text.Replace(keyword, "");
                    int versionDec = AppUtil.CalculateDecimalVersion(versionStr);

                    // PCに導入されているGnuPGのバージョンが2.3.4以上の場合は true
                    AppLogUtil.OutputLogDebug(string.Format("Installed GnuPG: version {0}", versionStr));
                    return (versionDec >= 20304);
                }
            }
            AppLogUtil.OutputLogDebug("GnuPG is not installed yet");
            return false;
        }

        private static string[] TextArrayOfResponse(string response)
        {
            return Regex.Split(response, "\r\n|\n");
        }

        private static bool WriteStringToFile(string contents, string filePath)
        {
            try {
                File.WriteAllText(filePath, contents);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winProcess.WriteStringToFile exception:\n{0}", e.Message));
                return false;
            }
        }
    }
}
