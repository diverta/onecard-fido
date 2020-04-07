using MaintenanceToolCommon;
using System;
using System.IO;
using System.Reflection;

namespace MaintenanceToolGUI
{
    public class ToolDFUImage
    {
        //
        // nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
        // .dat=256バイト、.bin=512Kバイトと見積っています。
        //
        private byte[] nrf52_app_dat = new byte[256];
        private byte[] nrf52_app_bin = new byte[524288];
        private byte[] nrf52_app_zip = new byte[524288];

        public int nrf52_app_dat_size { get; set; }
        public int nrf52_app_bin_size { get; set; }
        public int nrf52_app_zip_size { get; set; }

        // 更新イメージファイルのリソース名称
        private string DFUImageResourceName;

        // リソース名称検索用キーワード
        private const string ResourceNamePrefix = "MaintenanceToolGUI.Resources.app_dfu_package.";
        private const string ResourceNameSuffix = ".zip";

        public ToolDFUImage()
        {
            // ファームウェア更新イメージファイル名を取得
            GetDFUImageFileResourceName();

            // ファームウェア更新イメージ(.zip)を配列に読込
            ReadDFUImage();
        }

        public string GetUpdateVersionFromDFUImage()
        {
            // ファームウェア更新イメージ名称から、更新バージョンを取得
            return ExtractUpdateVersion(DFUImageResourceName);
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
            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream stream = assembly.GetManifestResourceStream(DFUImageResourceName);
            if (stream == null) {
                return;
            }
            AppCommon.OutputLogDebug(string.Format("ToolDFUImage.ReadDFUImage: DFUImageResourceName={0} Length={1}",
                DFUImageResourceName, stream.Length));

            try {
                // リソースファイルを配列に読込
                nrf52_app_zip_size = stream.Read(nrf52_app_zip, 0, (int)stream.Length);
                AppCommon.OutputLogDebug(string.Format("ToolDFUImage.ReadDFUImage: read {0} bytes",
                    nrf52_app_zip_size));

                // リソースファイルを閉じる
                stream.Close();

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolDFUImage.ReadDFUImage: {0}", e.Message));
                nrf52_app_zip_size = 0;
            }
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
