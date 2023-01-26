using MaintenanceToolApp.CommonProcess;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    public class DFUImageData
    {
        //
        // nRF5340アプリケーションファームウェアのバイナリーイメージを保持。
        // .bin=512Kバイトと見積っています。
        //
        public byte[] NRF53AppBin = new byte[0];
        public int NRF53AppBinSize { get; set; }

        // 更新イメージファイルのハッシュ値
        public byte[] SHA256Hash = new byte[32];

        //
        // nRF52840アプリケーションファームウェアのバイナリーイメージを保持。
        // .dat=256バイト、.bin=512Kバイトと見積っています。
        //
        public byte[] NRF52AppDat = Array.Empty<byte>();
        public byte[] NRF52AppBin = Array.Empty<byte>();
        public int NRF52AppDatSize { get; set; }
        public int NRF52AppBinSize { get; set; }

        // 更新イメージファイルのリソース名称
        public string DFUImageResourceName;

        // 更新イメージファイルのバージョン文字列
        public string UpdateVersion;

        // リソース名称検索用キーワード
        public const string ResourceNamePrefix = "app_update.";
        public const string ResourceNameSuffix = ".bin";
        public const string ResourceNamePrefixFor52 = "appkg.";
        public const string ResourceNameSuffixFor52 = ".zip";

        public DFUImageData()
        {
            DFUImageResourceName = string.Empty;
            UpdateVersion = string.Empty;
        }

        public static string ResourceName()
        {
            return string.Format("{0}.Resources.", AppUtil.GetAppBundleNameString());
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
            string UpdateVersion = Instance.GetUpdateVersionFromDFUImage(versionInfoData.HWRev);

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
            string minVersionForDFU;
            if (CurrentVersionIsUnavailableForDFU(versionInfoData.HWRev, currentVersionDec, out minVersionForDFU)) {
                string informative = string.Format(AppCommon.MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE, UpdateVersion, minVersionForDFU);
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

            // ファームウェア更新イメージ(.bin or .zip)を配列に読込
            if (ReadDFUImage() == false) {
                return false;
            }

            // nRF52固有対応
            // 書庫を解析し、内包されているイメージを抽出
            if (BoardIsNRF52(boardname)) {
                return ExtractDFUImage();
            }

            // イメージからSHA-256ハッシュを抽出
            return ExtractImageHashSha256();
        }

        public string GetUpdateVersionFromDFUImage(string boardname)
        {
            // ファームウェア更新イメージ名称から、更新バージョンを取得
            if (BoardIsNRF52(boardname)) {
                // nRF52固有対応
                return ExtractUpdateVersionFor52(ImageDataRef.DFUImageResourceName);

            } else {
                return ExtractUpdateVersion(ImageDataRef.DFUImageResourceName);
            }
        }

        private bool GetDFUImageFileResourceName(string boardname)
        {
            // リソース名称を初期化
            ImageDataRef.DFUImageResourceName = "";

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // nRF53用のイメージかどうか判定
                if (StartsWithResourceNameForNRF53(boardname, resName)) {
                    ImageDataRef.DFUImageResourceName = resName;
                    return true;
                }

                // nRF52用のイメージかどうか判定
                if (StartsWithResourceNameForNRF52(boardname, resName)) {
                    ImageDataRef.DFUImageResourceName = resName;
                    return true;
                }
            }

            return false;
        }

        private static bool StartsWithResourceNameForNRF53(string boardname, string resName)
        {
            // リソース名が
            // "<bundleName>.Resources.app_update.<boardname>."
            // という名称で始まっている場合は、
            // ファームウェア更新イメージファイルと判定
            string prefix = string.Format("{0}{1}{2}.", DFUImageData.ResourceName(), DFUImageData.ResourceNamePrefix, boardname);
            return resName.StartsWith(prefix);
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
                ImageDataRef.NRF53AppBin = new byte[stream.Length];
                ImageDataRef.NRF53AppBinSize = stream.Read(ImageDataRef.NRF53AppBin, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();

                // 読込長とバッファ長が異なる場合
                if (ImageDataRef.NRF53AppBinSize != ImageDataRef.NRF53AppBin.Length) {
                    AppLogUtil.OutputLogError(string.Format("ToolBLEDFUImage.ReadDFUImage: Read size {0} bytes, but image buffer size {1} bytes",
                        ImageDataRef.NRF53AppBinSize, ImageDataRef.NRF53AppBin.Length));
                    return false;
                }
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
            string replaced = resName.Replace(DFUImageData.ResourceName(), "").Replace(DFUImageData.ResourceNamePrefix, "").Replace(DFUImageData.ResourceNameSuffix, "");
            string[] elem = replaced.Split('.');
            if (elem.Length != 4) {
                return UpdateVersion;
            }

            // 抽出後の文字列を、基板名とバージョン文字列に分ける
            // 例：PCA10095.0.4.1 --> PCA10095, 0.4.1
            string boardname = elem[0];
            UpdateVersion = string.Format("{0}.{1}.{2}", elem[1], elem[2], elem[3]);

            // ログ出力
            string fname = resName.Replace(DFUImageData.ResourceName(), "");
            AppLogUtil.OutputLogDebug(string.Format("DFU image for nRF53: Firmware version {0}, board name {1}",
                UpdateVersion, boardname));
            AppLogUtil.OutputLogDebug(string.Format("DFU image for nRF53: {0}({1} bytes)",
                fname, ImageDataRef.NRF53AppBinSize
                ));

            return UpdateVersion;
        }

        private static bool CurrentVersionIsUnavailableForDFU(string boardname, int currentVersionDec, out string minVersionForDFU)
        {
            if (BoardIsNRF52(boardname)) {
                minVersionForDFU = "0.3.0";
                return (currentVersionDec < DFUProcessConst.DFU_UPD_TARGET_APP_VERSION_FOR_52);
            } else {
                minVersionForDFU = "0.4.0";
                return (currentVersionDec < DFUProcessConst.DFU_UPD_TARGET_APP_VERSION);
            }
        }

        //
        // nRF52固有対応
        //
        // DFU対象ファイル名
        private const string NRF52_APP_DAT_FILE_NAME = "nrf52840_xxaa.dat";
        private const string NRF52_APP_BIN_FILE_NAME = "nrf52840_xxaa.bin";

        private static bool StartsWithResourceNameForNRF52(string boardname, string resName)
        {
            // リソース名が
            // "<bundleName>.Resources.appkg.<boardname>."
            // という名称で始まっている場合は、
            // ファームウェア更新イメージファイルと判定
            string prefix = string.Format("{0}{1}{2}.", DFUImageData.ResourceName(), DFUImageData.ResourceNamePrefixFor52, boardname);
            return resName.StartsWith(prefix);
        }

        private static bool BoardIsNRF52(string boardname)
        {
            return boardname.StartsWith("PCA10059");
        }

        private bool ExtractDFUImage()
        {
            // 例外抑止
            if (ImageDataRef.NRF53AppBinSize == 0) {
                return false;
            }

            // .zip書庫ファイルを解析し、内包されている.bin/.datイメージを抽出
            int i = 0;
            while (i < ImageDataRef.NRF53AppBinSize) {
                if (ImageDataRef.NRF53AppBin[i + 0] == 0x50 && ImageDataRef.NRF53AppBin[i + 1] == 0x4B &&
                    ImageDataRef.NRF53AppBin[i + 2] == 0x03 && ImageDataRef.NRF53AppBin[i + 3] == 0x04) {
                    // 書庫エントリーのヘッダー（50 4B 03 04）が見つかった場合
                    i += ParseImage(ImageDataRef.NRF53AppBin, i);
                } else {
                    i++;
                }
            }
            return true;
        }

        private int ParseImage(byte[] data, int index)
        {
            // ファイルのサイズ
            int offset = 18;
            int compressedSize = AppUtil.ToInt32(data, index + offset, false);
            offset += 8;

            // ファイル名のサイズ
            int filenameSize = AppUtil.ToInt16(data, index + offset, false);
            offset += 2;

            // コメントのサイズ
            int commentSize = AppUtil.ToInt16(data, index + offset, false);
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
                ImageDataRef.NRF52AppDat = data.Skip(index + offset).Take(compressedSize).ToArray();
                ImageDataRef.NRF52AppDatSize = compressedSize;

            } else if (fileNameStr.Equals(NRF52_APP_BIN_FILE_NAME)) {
                // .binファイルのバイナリーイメージを配列に格納
                ImageDataRef.NRF52AppBin = data.Skip(index + offset).Take(compressedSize).ToArray();
                ImageDataRef.NRF52AppBinSize = compressedSize;
            }

            // 書庫エントリーのサイズを戻す
            offset += compressedSize;
            return offset;
        }

        public string ExtractUpdateVersionFor52(string resName)
        {
            // バージョン文字列を初期化
            string UpdateVersion = "";
            if (resName.Equals("")) {
                return UpdateVersion;
            }
            if (resName.EndsWith(DFUImageData.ResourceNameSuffixFor52) == false) {
                return UpdateVersion;
            }

            // リソース名称文字列から、バージョン文字列だけを抽出
            string replaced = resName.Replace(DFUImageData.ResourceName(), "").Replace(DFUImageData.ResourceNamePrefixFor52, "").Replace(DFUImageData.ResourceNameSuffixFor52, "");
            string[] elem = replaced.Split('.');
            if (elem.Length != 4) {
                return UpdateVersion;
            }

            // 抽出後の文字列を、基板名とバージョン文字列に分ける
            // 例：PCA10059_02.0.2.11 --> PCA10059_02, 0.2.11
            string boardname = elem[0];
            UpdateVersion = string.Format("{0}.{1}.{2}", elem[1], elem[2], elem[3]);

            // ログ出力
            AppLogUtil.OutputLogDebug(string.Format("DFU image for nRF52: Firmware version {0}, board name {1}",
                UpdateVersion, boardname));
            AppLogUtil.OutputLogDebug(string.Format("DFU image for nRF52: {0}({1} bytes), {2}({3} bytes)",
                NRF52_APP_DAT_FILE_NAME, ImageDataRef.NRF52AppDatSize,
                NRF52_APP_BIN_FILE_NAME, ImageDataRef.NRF52AppBinSize
                ));

            return UpdateVersion;
        }
    }
}
