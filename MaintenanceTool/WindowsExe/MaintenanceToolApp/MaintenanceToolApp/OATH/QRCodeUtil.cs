using System;
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
            Window MainWindow = Application.Current.MainWindow;

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
    }
}
