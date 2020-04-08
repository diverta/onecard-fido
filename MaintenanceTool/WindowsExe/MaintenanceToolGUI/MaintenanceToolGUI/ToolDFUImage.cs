using MaintenanceToolCommon;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

namespace MaintenanceToolGUI
{
    public class ToolDFUImage
    {
        //
        // nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
        // .dat=256バイト、.bin=512Kバイトと見積っています。
        //
        private byte[] nrf52AppDat;
        private byte[] nrf52AppBin;
        private byte[] nrf52AppZip = new byte[524288];

        public int nrf52AppDatSize { get; set; }
        public int nrf52AppBinSize { get; set; }
        public int nrf52AppZipSize { get; set; }

        // 更新イメージファイルのリソース名称
        private string DFUImageResourceName;

        // リソース名称検索用キーワード
        private const string ResourceNamePrefix = "MaintenanceToolGUI.Resources.app_dfu_package.";
        private const string ResourceNameSuffix = ".zip";

        // DFU対象ファイル名
        private const string NRF52_APP_DAT_FILE_NAME = "nrf52840_xxaa.dat";
        private const string NRF52_APP_BIN_FILE_NAME = "nrf52840_xxaa.bin";
        private const string NRF52_APP_ZIP_FILE_NAME = "app_dfu_package";

        public ToolDFUImage()
        {
            // ファームウェア更新イメージファイル名を取得
            GetDFUImageFileResourceName();

            // ファームウェア更新イメージ(.zip)を配列に読込
            ReadDFUImage();

            // 書庫を解析し、内包されているイメージを抽出
            ExtractDFUImage();
        }

        public string GetUpdateVersionFromDFUImage()
        {
            // ファームウェア更新イメージ名称から、更新バージョンを取得
            string UpdateVersion = ExtractUpdateVersion(DFUImageResourceName);

            // ログ出力
            AppCommon.OutputLogDebug(string.Format("ToolDFUImage: Firmware version {0}", UpdateVersion));
            AppCommon.OutputLogDebug(string.Format("ToolDFUImage: {0}({1} bytes), {2}({3} bytes)",
                NRF52_APP_DAT_FILE_NAME, nrf52AppDatSize,
                NRF52_APP_BIN_FILE_NAME, nrf52AppBinSize
                ));
            return UpdateVersion;
        }

        private void GetDFUImageFileResourceName()
        {
            // リソース名称を初期化
            DFUImageResourceName = "";

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolGUI.Resources.app_dfu_package."
                // という名称で始まっている場合は、
                // ファームウェア更新イメージファイルと判定
                if (resName.StartsWith(ResourceNamePrefix)) {
                    DFUImageResourceName = resName;
                }
            }
        }

        private void ReadDFUImage()
        {
            // ファイルサイズをゼロクリア
            nrf52AppDatSize = 0;
            nrf52AppBinSize = 0;
            nrf52AppZipSize = 0;

            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream stream = assembly.GetManifestResourceStream(DFUImageResourceName);
            if (stream == null) {
                return;
            }

            try {
                // リソースファイルを配列に読込
                nrf52AppZipSize = stream.Read(nrf52AppZip, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolDFUImage.ReadDFUImage: {0}", e.Message));
            }
        }

        private void ExtractDFUImage()
        {
            // 例外抑止
            if (nrf52AppZipSize == 0) {
                return;
            }

            // .zip書庫ファイルを解析し、内包されている.bin/.datイメージを抽出
            int i = 0;
            while (i < nrf52AppZipSize) {
                if (nrf52AppZip[i + 0] == 0x50 && nrf52AppZip[i + 1] == 0x4B &&
                    nrf52AppZip[i + 2] == 0x03 && nrf52AppZip[i + 3] == 0x04) {
                    // 書庫エントリーのヘッダー（50 4B 03 04）が見つかった場合
                    i += ParseImage(nrf52AppZip, i);
                } else {
                    i++;
                }
            }
        }

        private int ParseImage(byte[] data, int index)
        {
            // ファイルのサイズ
            int offset = 18;
            int compressedSize = AppCommon.ToInt32(data, index + offset, false);
            offset += 8;

            // ファイル名のサイズ
            int filenameSize = AppCommon.ToInt16(data, index + offset, false);
            offset += 2;

            // コメントのサイズ
            int commentSize = AppCommon.ToInt16(data, index + offset, false);
            offset += 2;

            // ファイル名
            byte[] file_name = data.Skip(index + offset).Take(filenameSize).ToArray();
            string fileNameStr = Encoding.UTF8.GetString(file_name);
            offset += filenameSize;

            // コメント
            offset += commentSize;

            // ファイルの内容
            if (fileNameStr.Equals(NRF52_APP_DAT_FILE_NAME)) {
                // .datファイルのバイナリーイメージを配列に格納
                nrf52AppDat = data.Skip(index + offset).Take(compressedSize).ToArray();
                nrf52AppDatSize = compressedSize;

            } else if (fileNameStr.Equals(NRF52_APP_BIN_FILE_NAME)) {
                // .binファイルのバイナリーイメージを配列に格納
                nrf52AppBin = data.Skip(index + offset).Take(compressedSize).ToArray();
                nrf52AppBinSize = compressedSize;
            }

            // 書庫エントリーのサイズを戻す
            offset += compressedSize;
            return offset;
        }

        private string ExtractUpdateVersion(string resName)
        {
            // バージョン文字列を初期化
            string UpdateVersion = "";
            if (resName.Equals("")) {
                return UpdateVersion;
            }
            if (resName.EndsWith(ResourceNameSuffix) == false) {
                return UpdateVersion;
            }

            // リソース名称文字列から、バージョン文字列だけを抽出
            UpdateVersion = resName.Replace(ResourceNamePrefix, "").Replace(ResourceNameSuffix, "");
            return UpdateVersion;
        }
    }
}
