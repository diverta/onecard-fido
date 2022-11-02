using System;
using System.Collections.Generic;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    public class PIVSettingDataObjects
    {
        // PIVデータオブジェクトを保持
        private readonly Dictionary<UInt32, byte[]> DataObject = new Dictionary<UInt32, byte[]>();

        public void Set(UInt32 objectId, byte[] objectData)
        {
            // データを連想配列にコピー
            byte[] b = new byte[objectData.Length];
            Array.Copy(objectData, b, objectData.Length);
            DataObject[objectId] = b;
        }

        public byte[] Get(UInt32 objectId)
        {
            if (DataObject.ContainsKey(objectId)) {
                return DataObject[objectId];
            } else {
                return Array.Empty<byte>();
            }
        }
    }

    internal class PIVSettingDataProcess
    {
        // 後続処理の参照を保持
        public delegate void HandlerNextProcess();
        private HandlerNextProcess HandlerNextProcessRef = null!;

        // このクラスのインスタンス
        private static readonly PIVSettingDataProcess Instance = new PIVSettingDataProcess();

        // 取得対象オブジェクトIDを保持
        private UInt32 ObjectIdToFetch { get; set; }

        // PIV設定情報クラスの参照を保持
        private PIVSettingDataObjects PIVSettingDataObjectsRef = null!;

        //
        // PIVデータオブジェクト照会
        //
        public static void DoRequestPIVGetDataObject(UInt32 objectId, PIVSettingDataObjects settings, HandlerNextProcess handlerNextProcess)
        {
            // 後続処理の参照を保持
            Instance.HandlerNextProcessRef = handlerNextProcess;

            // 後続処理の参照を保持
            Instance.PIVSettingDataObjectsRef = settings;

            // 取得対象オブジェクトをAPDUに格納
            Instance.ObjectIdToFetch = objectId;
            byte[] apdu = GetPivInsGetApdu(objectId);

            // コマンドを実行
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_GET_DATA, 0x3f, 0xff, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePIVGetDataObject);
        }

        private static void DoResponsePIVGetDataObject(bool success, byte[] responseData, UInt16 responseSW)
        {
            // インスタンスで保持されている変数
            UInt32 objectId = Instance.ObjectIdToFetch;
            PIVSettingDataObjects settings = Instance.PIVSettingDataObjectsRef;
            HandlerNextProcess handlerNextProcess = Instance.HandlerNextProcessRef;

            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                // 処理失敗ログを出力（エラーではなく警告扱いとする）
                AppLogUtil.OutputLogWarn(string.Format(AppCommon.MSG_ERROR_PIV_DATA_OBJECT_GET_FAILED, objectId));
                // ブランクデータをPIV設定情報クラスに設定
                settings.Set(objectId, Array.Empty<byte>());

            } else {
                // 処理成功ログを出力
                AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_PIV_DATA_OBJECT_GET, objectId));
                // 取得したデータをPIV設定情報クラスに設定
                settings.Set(objectId, responseData);
            }

            // 後続処理が指定されている場合は実行
            if (handlerNextProcess != null) {
                handlerNextProcess();
            }
        }

        private static byte[] GetPivInsGetApdu(UInt32 objectId)
        {
            byte[] apdu = new byte[5];
            int offset = 0;

            // 0x5c: TAG_DATA_OBJECT
            apdu[offset++] = 0x5c;

            // オブジェクト長の情報を設定
            apdu[offset++] = 3;
            apdu[offset++] = (byte)((objectId >> 16) & 0x000000ff);
            apdu[offset++] = (byte)((objectId >> 8) & 0x000000ff);
            apdu[offset++] = (byte)(objectId & 0x000000ff);
            return apdu;
        }
    }
}
