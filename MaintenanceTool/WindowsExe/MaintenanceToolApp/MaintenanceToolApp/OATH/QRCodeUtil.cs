using OpenCvSharp;
using OpenCvSharp.Extensions;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows;
using System.Windows.Media;
using ToolAppCommon;

namespace MaintenanceToolApp.OATH
{
    internal class QRCodeUtil
    {
        // スクリーンショット画像を保持
        private Bitmap? BitmapScreenShot { get; set; }

        // スキャンしたQRコードを保持
        private string? QRCodeString { get; set; }

        // QRコードの解析結果を保持
        private Dictionary<string, string> ParsedQRCodeInfo { get; set; } = new Dictionary<string, string>();

        //
        // 公開用関数
        //
        public bool ScanQRCodeFromScreenShot()
        {
            // デスクトップのスクリーンショットを取得し、イメージを抽出
            if (TakeScreenShotFromRectangle(QRCodeUtil.GetFullScreenRectangle()) == false) {
                return false;
            }

            // イメージからQRコードをキャプチャーし、メッセージを抽出
            if (ExtractQRMessage() == false) {
                return false;
            }

            // 抽出されたメッセージを解析
            if (ParseQRMessage() == false) {
                return false;
            }

            return true;
        }

        public bool TakeScreenShotFromRectangle(Rectangle rectangle)
        {
            try {
                // 指定矩形領域のスクリーンショットを取得
                BitmapScreenShot = new Bitmap(rectangle.Width, rectangle.Height);
                Graphics graphics = Graphics.FromImage(BitmapScreenShot);

                // スクリーンショットから、イメージを抽出
                graphics.CopyFromScreen(rectangle.X, rectangle.Y, 0, 0, rectangle.Size);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("TakeScreenShotFromRectangle fail: {0}", e.Message));
                return false;
            }
        }

        public bool SaveBitmapScreenShotToFile()
        {
            // 例外抑止
            if (BitmapScreenShot == null) {
                AppLogUtil.OutputLogError("SaveBitmapFileOfScreenShot fail: Screenshot bitmap not exist");
                return false;
            }

            try {
                // アプリケーションのログディレクトリー配下に、スクリーンショットの画像ファイルを出力
                string tempFileName = string.Format("{0}\\{1}{2}.jpg", AppLogUtil.OutputLogFileDirectoryPath(), DateTime.Now.ToString("hhmmss"), DateTime.Now.Millisecond.ToString());
                BitmapScreenShot.Save(tempFileName, ImageFormat.Jpeg);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("SaveBitmapFileOfScreenShot fail: {0}", e.Message));
                return false;
            }
        }

        public static Rectangle GetFullScreenRectangle()
        {
            // メイン画面のウィンドウ
            System.Windows.Window MainWindow = Application.Current.MainWindow;

            // DIPを物理的なピクセルに変換するための係数を取得
            PresentationSource MainWindowPresentationSource = PresentationSource.FromVisual(MainWindow);
            Matrix m = MainWindowPresentationSource.CompositionTarget.TransformToDevice;
            double dpiWidthFactor = m.M11;
            double dpiHeightFactor = m.M22;

            // DIPのスクリーンサイズに係数を掛け、スクリーンの解像度を取得
            double ScreenHeight = SystemParameters.PrimaryScreenHeight * dpiHeightFactor;
            double ScreenWidth = SystemParameters.PrimaryScreenWidth * dpiWidthFactor;

            // 画面全体の矩形領域を取得
            Rectangle rectangle = new Rectangle(0, 0, (int)ScreenWidth, (int)ScreenHeight);
            return rectangle;
        }

        //
        // 内部処理
        //
        private bool ExtractQRMessage()
        {
            // 例外抑止
            if (BitmapScreenShot == null) {
                AppLogUtil.OutputLogError("ExtractQRMessage fail: Screenshot bitmap not exist");
                return false;
            }

            try {
                // QRコードを解析し、テキストを取得
                Mat qrImage = BitmapConverter.ToMat(BitmapScreenShot);
                QRCodeDetector detector = new QRCodeDetector();
                QRCodeString = detector.DetectAndDecode(qrImage, out Point2f[] points);

                // テキストが取得できない場合は false
                if (QRCodeString.Equals(string.Empty)) {
                    AppLogUtil.OutputLogDebug("QR code not detected");
                    return false;
                }

                AppLogUtil.OutputLogDebug("QR code detected from screen");
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("ExtractQRMessage fail: {0}", e.Message));
                return false;
            }
        }

        private bool ParseQRMessage()
        {
            // 例外抑止
            if (QRCodeString == null || QRCodeString.Equals(string.Empty)) {
                AppLogUtil.OutputLogError("ParseQRMessage fail: QR message text not exist");
                return false;
            }

            // 配列を初期化
            ParsedQRCodeInfo.Clear();

            // offsetを検索対象文字列の先頭に設定
            int offset = 0;

            // 検索用区切り文字列を設定
            string[] strings = { "://", "/", "?", "&" };
            int i = 0;
            while (i < strings.Length && offset < QRCodeString.Length) {
                // 検索用区切り文字が出現するまでの範囲を取得
                string separator = strings[i];
                int endIndex = GetEndIndex(QRCodeString, offset, separator);

                // 部分文字列を抽出し、連想配列に設定
                string substring = QRCodeString.Substring(offset, endIndex - offset);
                ExtractParameter(substring, i, ParsedQRCodeInfo);

                // offsetを検索対象文字列の位置に更新
                offset = endIndex + separator.Length;

                // & の場合は見つからなくなるまで検索を続ける
                if (separator.Equals("&") == false) {
                    i++;
                }
            }
            return true;
        }

        private int GetEndIndex(string message, int startIndex, string terminator)
        {
            int endIndex = message.IndexOf(terminator, startIndex);
            if (endIndex == -1) {
                endIndex = message.Length;
            }
            return endIndex;
        }

        private void ExtractParameter(string paramString, int paramNumber, Dictionary<string, string> dictionary)
        {
            switch (paramNumber) {
            case 0:
                // protocol
                dictionary.Add("protocol", paramString);
                break;
            case 1:
                // OATH method
                dictionary.Add("method", paramString);
                break;
            case 2:
                // OATH account
                ExtractAccountParameter(paramString, dictionary);
                break;
            default:
                // OATH parameter
                ExtractOathParameter(paramString, dictionary);
                break;
            }
        }

        private void ExtractAccountParameter(string paramString, Dictionary<string, string> dictionary)
        {
            string[] parameter = paramString.Split(":");
            string value;
            if (parameter.Length > 1) {
                value = parameter[1];
            } else {
                value = parameter[0];
            }
            dictionary.Add("account", value);
        }

        private void ExtractOathParameter(string paramString, Dictionary<string, string> dictionary)
        {
            // キーと値のペアでない場合は何もしない
            string[] parameter = GetSeparatedParameter(paramString, "=");
            if (parameter.Length != 2) {
                return;
            }

            // キーと値のペアを連想配列に設定
            dictionary.Add(parameter[0], parameter[1]);
        }

        private string[] GetSeparatedParameter(string parameterString, string separator)
        {
            // 戻り値の配列
            string[] parameters;

            // 区切り文字の位置を取得
            int endIndex = parameterString.IndexOf(separator);
            if (endIndex == -1) {
                // 区切り文字列が無い場合は単一値を戻す
                parameters = new string[] { parameterString };

            } else {
                // 区切り文字列の前後を配列で戻す
                string key = parameterString.Substring(0, endIndex);
                string value = parameterString.Substring(endIndex + separator.Length);
                parameters = new string[] { key, value };
            }

            return parameters;
        }
    }
}
