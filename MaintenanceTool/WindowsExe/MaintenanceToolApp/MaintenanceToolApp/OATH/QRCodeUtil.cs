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
        private Dictionary<string, string> ParsedQRCodeInfo { get; set; }

        // 解析時の検索開始インデックスを保持
        private int Offset = 0;

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

        public bool ExtractQRMessage()
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
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("ExtractQRMessage fail: {0}", e.Message));
                return false;
            }
        }

        public bool ParseQRMessage()
        {
            // 例外抑止
            if (QRCodeString == null || QRCodeString.Equals(string.Empty)) {
                AppLogUtil.OutputLogError("ParseQRMessage fail: QR message text not exist");
                return false;
            }

            // 配列を初期化
            ParsedQRCodeInfo = new Dictionary<string, string>();

            // offsetを検索対象文字列の先頭に設定
            Offset = 0;

            // 検索用区切り文字列を設定
            string[] strings = { "://", "/", "?", "&" };
            int i = 0;
            while (i < strings.Length && Offset < QRCodeString.Length) {
                // 検索用区切り文字が出現するまでの範囲を取得
                string separator = strings[i];
                int endIndex = GetEndIndex(QRCodeString, Offset, separator);

                // 部分文字列を抽出し、連想配列に設定
                string substring = QRCodeString.Substring(Offset, endIndex - Offset);
                ExtractParameter(substring, i, ParsedQRCodeInfo);

                // offsetを検索対象文字列の位置に更新
                Offset = endIndex + separator.Length;

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
            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(string.Format("ExtractParameter {0}:{1}", paramNumber, paramString));
        }
    }
}
