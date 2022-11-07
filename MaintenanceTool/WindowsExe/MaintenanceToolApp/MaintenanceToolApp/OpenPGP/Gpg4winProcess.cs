using System;
using System.Diagnostics;
using System.IO;
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
    }
}
