using MaintenanceToolApp.CommonProcess;
using System;
using System.IO;
using System.Reflection;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    public class DFUImageData
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
        public string DFUImageResourceName;

        // 更新イメージファイルのバージョン文字列
        public string UpdateVersion;

        // リソース名称検索用キーワード
        public const string ResourceName = "MaintenanceToolApp.Resources.";
        public const string ResourceNamePrefix = "app_update.";
        public const string ResourceNameSuffix = ".bin";

        public DFUImageData()
        {
            DFUImageResourceName = String.Empty;
            UpdateVersion = String.Empty;
        }
    }

    internal class DFUImage
    {
        // このクラスのインスタンス
        private static readonly DFUImage Instance = new DFUImage();

        // イメージデータを保持
        private readonly DFUImageData ImageDataRef = new DFUImageData();

        //
        // 外部公開用
        //
        public static bool CheckAndGetUpdateVersion(VersionInfoData versionInfoData, out string checkErrorCaption, out string checkErrorMessage, out DFUImageData imageData)
        {
            checkErrorCaption = string.Empty;
            checkErrorMessage = string.Empty;
            imageData = Instance.ImageDataRef;

            // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
            if (Instance.ReadBLEDFUImageFile(versionInfoData.HWRev) == false) {
                checkErrorCaption = AppCommon.MSG_DFU_IMAGE_NOT_AVAILABLE;
                checkErrorMessage = AppCommon.MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST;
                return false;
            }

            // ファームウェア更新イメージファイルから、更新バージョンを取得
            string UpdateVersion = Instance.GetUpdateVersionFromDFUImage();

            // 更新イメージファイル名からバージョンが取得できていない場合は利用不可
            if (UpdateVersion.Equals("")) {
                checkErrorCaption = AppCommon.MSG_DFU_IMAGE_NOT_AVAILABLE;
                checkErrorMessage = AppCommon.MSG_DFU_UPDATE_VERSION_UNKNOWN;
                return false;
            }

            // BLE経由で認証器の現在バージョンが取得できていない場合は利用不可
            string CurrentVersion = versionInfoData.FWRev;
            if (CurrentVersion.Equals("")) {
                checkErrorCaption = AppCommon.MSG_DFU_IMAGE_NOT_AVAILABLE;
                checkErrorMessage = AppCommon.MSG_DFU_CURRENT_VERSION_UNKNOWN;
                return false;
            }

            // 認証器の現在バージョンが、更新イメージファイルのバージョンより新しい場合は利用不可
            int currentVersionDec = AppUtil.CalculateDecimalVersion(CurrentVersion);
            int updateVersionDec = AppUtil.CalculateDecimalVersion(UpdateVersion);
            if (currentVersionDec > updateVersionDec) {
                string informative = string.Format(AppCommon.MSG_DFU_CURRENT_VERSION_ALREADY_NEW, CurrentVersion, UpdateVersion);
                checkErrorCaption = AppCommon.MSG_DFU_IMAGE_NOT_AVAILABLE;
                checkErrorMessage = informative;
                return false;
            }

            // 認証器の現在バージョンが、所定バージョンより古い場合は利用不可
            // （ブートローダーのバージョンが異なるため）
            if (currentVersionDec < DFUProcessConst.DFU_UPD_TARGET_APP_VERSION) {
                string informative = string.Format(AppCommon.MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE, UpdateVersion);
                checkErrorCaption = AppCommon.MSG_DFU_IMAGE_NOT_AVAILABLE;
                checkErrorMessage = informative;
                return false;
            }

            // 更新イメージのバージョン文字列を設定
            Instance.ImageDataRef.UpdateVersion = UpdateVersion;
            return true;
        }

        //
        // 内部処理
        //
        private bool ReadBLEDFUImageFile(string boardname)
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
            string UpdateVersion = ExtractUpdateVersion(ImageDataRef.DFUImageResourceName);
            return UpdateVersion;
        }

        private bool GetDFUImageFileResourceName(string boardname)
        {
            // リソース名称を初期化
            ImageDataRef.DFUImageResourceName = "";

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolApp.Resources.app_update.<boardname>."
                // という名称で始まっている場合は、
                // ファームウェア更新イメージファイルと判定
                string prefix = string.Format("{0}{1}{2}.", DFUImageData.ResourceName, DFUImageData.ResourceNamePrefix, boardname);
                if (resName.StartsWith(prefix)) {
                    ImageDataRef.DFUImageResourceName = resName;
                    return true;
                }
            }

            return false;
        }

        private bool ReadDFUImage()
        {
            // ファイルサイズをゼロクリア
            ImageDataRef.NRF53AppBinSize = 0;

            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream? stream = assembly.GetManifestResourceStream(ImageDataRef.DFUImageResourceName);
            if (stream == null) {
                return false;
            }

            try {
                // リソースファイルを配列に読込
                ImageDataRef.NRF53AppBinSize = stream.Read(ImageDataRef.NRF53AppBin, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("ToolBLEDFUImage.ReadDFUImage: {0}", e.Message));
                return false;
            }
        }

        private bool ExtractImageHashSha256()
        {
            // magicの値を抽出
            ulong magic = (ulong)AppUtil.ToInt32(ImageDataRef.NRF53AppBin, 0, false);

            // イメージヘッダー／データ長を抽出
            int image_header_size = AppUtil.ToInt32(ImageDataRef.NRF53AppBin, 8, false);
            int image_data_size = AppUtil.ToInt32(ImageDataRef.NRF53AppBin, 12, false);
            int image_size = image_header_size + image_data_size;

            // イメージヘッダーから、イメージTLVの開始位置を計算
            int tlv_info;
            if (magic == 0x96f3b83c) {
                tlv_info = image_size;
            } else {
                tlv_info = image_size + 4;
            }

            // イメージTLVからSHA-256ハッシュの開始位置を検出
            while (tlv_info < ImageDataRef.NRF53AppBinSize) {
                // タグ／長さを抽出
                int tag = AppUtil.ToInt16(ImageDataRef.NRF53AppBin, tlv_info, false);
                tlv_info += 2;
                int len = AppUtil.ToInt16(ImageDataRef.NRF53AppBin, tlv_info, false);
                tlv_info += 2;

                // SHA-256のタグであり、長さが32バイトであればデータをバッファにコピー
                if (tag == 0x10 && len == 0x20) {
                    Array.Copy(ImageDataRef.NRF53AppBin, tlv_info, ImageDataRef.SHA256Hash, 0, len);
                    return true;
                } else {
                    tlv_info += len;
                }
            }
            // SHA-256ハッシュが見つからなかった場合はエラー
            AppLogUtil.OutputLogError("ToolBLEDFUImage.ExtractImageHashSha256: SHA-256 hash of image not found");
            return false;
        }

        private string ExtractUpdateVersion(string resName)
        {
            // バージョン文字列を初期化
            string UpdateVersion = "";
            if (resName.Equals("")) {
                return UpdateVersion;
            }
            if (resName.EndsWith(DFUImageData.ResourceNameSuffix) == false) {
                return UpdateVersion;
            }

            // リソース名称文字列から、バージョン文字列だけを抽出
            string replaced = resName.Replace(DFUImageData.ResourceName, "").Replace(DFUImageData.ResourceNamePrefix, "").Replace(DFUImageData.ResourceNameSuffix, "");
            string[] elem = replaced.Split('.');
            if (elem.Length != 4) {
                return UpdateVersion;
            }

            // 抽出後の文字列を、基板名とバージョン文字列に分ける
            // 例：PCA10095.0.4.1 --> PCA10095, 0.4.1
            string boardname = elem[0];
            UpdateVersion = string.Format("{0}.{1}.{2}", elem[1], elem[2], elem[3]);

            // ログ出力
            string fname = resName.Replace(DFUImageData.ResourceName, "");
            AppLogUtil.OutputLogDebug(string.Format("ToolBLEDFUImage: Firmware version {0}, board name {1}",
                UpdateVersion, boardname));
            AppLogUtil.OutputLogDebug(string.Format("ToolBLEDFUImage: {0}({1} bytes)",
                fname, ImageDataRef.NRF53AppBinSize
                ));

            return UpdateVersion;
        }
    }
}
