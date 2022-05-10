using System;
using System.IO;
using System.Reflection;

namespace MaintenanceToolGUI
{
    public class ToolBLEDFUImage
    {
        //
        // nRF5340アプリケーションファームウェアのバイナリーイメージを保持。
        // .bin=512Kバイトと見積っています。
        //
        public byte[] NRF53AppBin = new byte[524288];
        public int NRF53AppBinSize { get; set; }

        // 更新イメージファイルのハッシュ値
        public byte[] SHA256Hash = new byte[32];

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

            // イメージからSHA-256ハッシュを抽出
            return ExtractImageHashSha256();
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

        private bool ExtractImageHashSha256()
        {
            // magicの値を抽出
            ulong magic = (ulong)AppCommon.ToInt32(NRF53AppBin, 0, false);

            // イメージヘッダー／データ長を抽出
            int image_header_size = AppCommon.ToInt32(NRF53AppBin, 8, false);
            int image_data_size = AppCommon.ToInt32(NRF53AppBin, 12, false);
            int image_size = image_header_size + image_data_size;

            // イメージヘッダーから、イメージTLVの開始位置を計算
            int tlv_info;
            if (magic == 0x96f3b83c) {
                tlv_info = image_size;
            } else {
                tlv_info = image_size + 4;
            }

            // イメージTLVからSHA-256ハッシュの開始位置を検出
            while (tlv_info < NRF53AppBinSize) {
                // タグ／長さを抽出
                int tag = AppCommon.ToInt16(NRF53AppBin, tlv_info, false);
                tlv_info += 2;
                int len = AppCommon.ToInt16(NRF53AppBin, tlv_info, false);
                tlv_info += 2;

                // SHA-256のタグであり、長さが32バイトであればデータをバッファにコピー
                if (tag == 0x10 && len == 0x20) {
                    Array.Copy(NRF53AppBin, tlv_info, SHA256Hash, 0, len);
                    return true;
                } else {
                    tlv_info += len;
                }
            }
            // SHA-256ハッシュが見つからなかった場合はエラー
            AppCommon.OutputLogError("ToolBLEDFUImage.ExtractImageHashSha256: SHA-256 hash of image not found");
            return false;
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
