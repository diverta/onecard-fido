using System.IO;
using System.Text.RegularExpressions;
using ToolAppCommon;

namespace MaintenanceToolApp.OpenPGP
{
    internal class Gpg4winUtility
    {
        //
        // ユーティリティー関数群
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

        public static string ExtractMainKeyIdFromResponse(string response)
        {
            // メッセージ文字列から鍵IDを抽出
            string keyid = string.Empty;

            // メッセージ検索用文字列
            string keyword = "pub   rsa2048";

            // 改行文字で区切られた文字列を分割
            string[] textArray = TextArrayOfResponse(response);

            // 分割されたメッセージの１件目について、鍵の機能を解析
            if (textArray[0].StartsWith(keyword)) {
                if (textArray[0].Contains("[C]")) {
                    // 分割されたメッセージの２件目、後ろから16バイトの文字列を、鍵IDとして抽出
                    int startIndex = textArray[1].Length - 16;
                    keyid = textArray[1].Substring(startIndex);
                }
            }

            // 抽出された鍵IDを戻す
            return keyid;
        }

        public static bool CheckIfSubKeysExistFromResponse(string response, bool transferred, string tempFolderPath)
        {
            // メッセージ検索用文字列
            string keyword1 = string.Format("{0}\\pubring.kbx", tempFolderPath);
            string keyword2 = transferred ? "ssb>  rsa2048" : "ssb   rsa2048";

            // 副鍵生成の有無を保持
            bool subKeyS = false;
            bool subKeyE = false;
            bool subKeyA = false;

            // メッセージ文字列から鍵一覧メッセージ（'gpg -K'実行結果）を抽出
            bool gpgKisAvailable = false;
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.StartsWith(keyword1)) {
                    // 'gpg -K'の実行結果が、メッセージ文字列中に存在すると判断
                    gpgKisAvailable = true;
                    continue;
                }
                // 'gpg -K'の実行結果を解析
                if (gpgKisAvailable) {
                    // 副鍵に関するメッセージを解析
                    if (text.StartsWith(keyword2)) {
                        // 副鍵の機能を解析
                        if (text.Contains("[S]")) {
                            subKeyS = true;
                        } else if (text.Contains("[E]")) {
                            subKeyE = true;
                        } else if (text.Contains("[A]")) {
                            subKeyA = true;
                        }
                    }
                }
            }

            // ３点の副鍵が揃っていれば true を戻す
            if (subKeyS && subKeyE && subKeyA) {
                return true;
            }

            // 揃っていない副鍵についてログを出力
            string str = transferred ? "transferred" : "added";
            if (subKeyS == false) {
                AppLogUtil.OutputLogDebug(string.Format("Sub key (for sign) not {0}", str));
            }
            if (subKeyE == false) {
                AppLogUtil.OutputLogDebug(string.Format("Sub key (for encrypt) not {0}", str));
            }
            if (subKeyA == false) {
                AppLogUtil.OutputLogDebug(string.Format("Sub key (for authenticate) not {0}", str));
            }

            // false を戻す
            return false;
        }

        public static bool CheckIfPubkeyAndBackupExist(string pubkeyFolderPath, string backupFolderPath)
        {
            // 公開鍵ファイルがエクスポート先に存在するかチェック
            if (CheckIfFileExist("public_key.pgp", pubkeyFolderPath) == false) {
                AppLogUtil.OutputLogError(AppCommon.MSG_ERROR_OPENPGP_EXPORT_PUBKEY_FAIL);
                return false;
            }

            // バックアップファイルがエクスポート先に存在するかチェック
            if (CheckIfFileExist("GNUPGHOME.tgz", backupFolderPath) == false) {
                AppLogUtil.OutputLogError(AppCommon.MSG_ERROR_OPENPGP_BACKUP_FAIL);
                return false;
            }

            return true;
        }

        private static bool CheckIfFileExist(string filename, string inFolderPath)
        {
            // 指定のフォルダー配下にファイルが存在している場合は true
            string filePath = string.Format("{0}\\{1}", inFolderPath, filename);
            return File.Exists(filePath);
        }

        public static bool CheckIfSubKeyAlreadyStoredFromResponse(string response)
        {
            // メッセージ検索用文字列
            string keyword = "such a key has already been stored on the card!";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keyword)) {
                    return true;
                }
            }
            return false;
        }

        public static bool CheckIfNoSubKeyExistFromResponse(string response)
        {
            // ステータス開始行の有無を保持
            bool header = false;

            // 副鍵の有無を保持
            bool subKeyS = false;
            bool subKeyE = false;
            bool subKeyA = false;

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains("Reader ...........:")) {
                    header = true;
                } else if (text.Contains("Signature key")) {
                    subKeyS = text.Contains("[none]");
                } else if (text.Contains("Encryption key")) {
                    subKeyE = text.Contains("[none]");
                } else if (text.Contains("Authentication key")) {
                    subKeyA = text.Contains("[none]");
                }
            }

            // ３点の副鍵が削除されていれば true を戻す
            return (header && subKeyS && subKeyE && subKeyA);
        }

        public static bool CheckIfCardErrorFromResponse(string response)
        {
            // メッセージ検索用文字列
            string keyword = "selecting card failed";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keyword)) {
                    return true;
                }
            }
            return false;
        }

        private static string[] TextArrayOfResponse(string response)
        {
            return Regex.Split(response, "\r\n|\n");
        }
    }
}
