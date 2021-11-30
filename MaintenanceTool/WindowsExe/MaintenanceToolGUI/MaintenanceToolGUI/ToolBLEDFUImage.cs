using MaintenanceToolCommon;
using System;
using System.IO;
using System.Reflection;

namespace MaintenanceToolGUI
{
    class ToolBLEDFUImage
    {
        //
        // nRF5340アプリケーションファームウェアのバイナリーイメージを保持。
        // .bin=512Kバイトと見積っています。
        //
        public byte[] NRF53AppBin = new byte[524288];
        public int NRF53AppBinSize { get; set; }

        // 更新イメージファイルのリソース名称
        private string DFUImageResourceName;

        // リソース名称検索用キーワード
        private const string ResourceName = "MaintenanceToolGUI.Resources.";
        private const string ResourceNamePrefix = "app_update.";
        private const string ResourceNameSuffix = ".bin";

        public ToolBLEDFUImage()
        {
        }

        public bool ReadBLEDFUImageFile(string boardname)
        {
            // ファームウェア更新イメージファイル名を取得
            if (GetDFUImageFileResourceName(boardname) == false) {
                return false;
            }

            // ファームウェア更新イメージ(.bin)を配列に読込
            if (ReadDFUImage() == false) {
                return false;
            }

            return true;
        }

        public string GetUpdateVersionFromDFUImage()
        {
            // ファームウェア更新イメージ名称から、更新バージョンを取得
            string UpdateVersion = ExtractUpdateVersion(DFUImageResourceName);
            return UpdateVersion;
        }

        private bool GetDFUImageFileResourceName(string boardname)
        {
            // リソース名称を初期化
            DFUImageResourceName = "";

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolGUI.Resources.app_update.<boardname>."
                // という名称で始まっている場合は、
                // ファームウェア更新イメージファイルと判定
                string prefix = string.Format("{0}{1}{2}.", ResourceName, ResourceNamePrefix, boardname);
                if (resName.StartsWith(prefix)) {
                    DFUImageResourceName = resName;
                    return true;
                }
            }

            return false;
        }

        private bool ReadDFUImage()
        {
            // ファイルサイズをゼロクリア
            NRF53AppBinSize = 0;

            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream stream = assembly.GetManifestResourceStream(DFUImageResourceName);
            if (stream == null) {
                return false;
            }

            try {
                // リソースファイルを配列に読込
                NRF53AppBinSize = stream.Read(NRF53AppBin, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolBLEDFUImage.ReadDFUImage: {0}", e.Message));
                return false;
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
            string replaced = resName.Replace(ResourceName, "").Replace(ResourceNamePrefix, "").Replace(ResourceNameSuffix, "");
            string[] elem = replaced.Split('.');
            if (elem.Length != 4) {
                return UpdateVersion;
            }

            // 抽出後の文字列を、基板名とバージョン文字列に分ける
            // 例：PCA10095.0.4.1 --> PCA10095, 0.4.1
            string boardname = elem[0];
            UpdateVersion = string.Format("{0}.{1}.{2}", elem[1], elem[2], elem[3]);

            // ログ出力
            string fname = resName.Replace(ResourceName, "");
            AppCommon.OutputLogDebug(string.Format("ToolBLEDFUImage: Firmware version {0}, board name {1}",
                UpdateVersion, boardname));
            AppCommon.OutputLogDebug(string.Format("ToolBLEDFUImage: {0}({1} bytes)",
                fname, NRF53AppBinSize
                ));

            return UpdateVersion;
        }
    }
}
