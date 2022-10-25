using System;
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

        private static string[] TextArrayOfResponse(string response)
        {
            return Regex.Split(response, "\r\n|\n");
        }

        public static bool WriteStringToFile(string contents, string filePath)
        {
            try {
                File.WriteAllText(filePath, contents);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Gpg4winUtility.WriteStringToFile exception:\n{0}", e.Message));
                return false;
            }
        }
    }
}
