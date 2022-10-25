using System;
using System.Diagnostics;
using System.IO;
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
                AppLogUtil.OutputLogError(string.Format("OpenPGPUtil.MakeTempFolder exception:\n{0}", e.Message));
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
                AppLogUtil.OutputLogError(string.Format("OpenPGPUtil.RemoveTempFolder exception:\n{0}", e.Message));
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
        // ユーティリティー
        //
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
    }
}
