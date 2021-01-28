using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    class ToolDFUProcess
    {
        // DFUデバイスクラスの参照を保持
        private DFUDevice DFUDeviceRef;

        // 更新イメージクラスの参照を保持
        private ToolDFUImage ToolDFUImageRef;

        // DFU転送処理完了時のイベント
        public delegate void DFUProcessTerminatedEventHandler(bool success);
        public event DFUProcessTerminatedEventHandler DFUProcessTerminatedEvent;

        public ToolDFUProcess(DFUDevice d, ToolDFUImage i)
        {
            // クラスの参照を保持
            DFUDeviceRef = d;
            ToolDFUImageRef = i;

            // イベントの登録
            DFUDeviceRef.NotifyDFUResponseEvent += new DFUDevice.NotifyDFUResponseEventHandler(OnReceiveDFUResponse);
        }

        public void PerformDFU()
        {
            // DFU対象デバイスの通知設定
            SendSetReceiptRequest();
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            DFUProcessTerminatedEvent(success);
        }

        //
        // 転送準備処理
        //
        private void SendSetReceiptRequest()
        {
            // SET RECEIPTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveSetReceiptRequest(byte[] response)
        {
            // DFU対象デバイスからMTUを取得
            SendGetMtuRequest();
        }

        private void SendGetMtuRequest()
        {
            // GET MTUコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_MTU_GET, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveGetMtuRequest(byte[] response)
        {
            // レスポンスからMTUを取得（4〜5バイト目、リトルエンディアン）
            MTU = AppCommon.ToInt16(response, 3, false);

            // DATイメージ転送処理の開始
            // １回あたりの送信データ最大長を取得
            SendSelectObjectRequest(NRFDfuConst.NRF_DFU_BYTE_OBJ_INIT_CMD);
        }

        //
        // イメージ転送処理で使用する変数
        //
        private ToolDFUUtil toolDFUUtil;
        private byte ObjectType;
        private int MTU;
        private int MaxCreateSize;
        private int AlreadySent;
        private int RemainingToSend;
        private int SizeToSend;

        //
        // イメージ転送処理（DAT／BIN）
        //
        private void SendSelectObjectRequest(byte type)
        {
            // 転送対象オブジェクトの区分（DAT／BIN）を保持
            ObjectType = type;

            // SELECT OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_OBJECT_SELECT, ObjectType, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveSelectObjectRequest(byte[] response)
        {
            // レスポンスからMaxCreateSizeを取得（4〜7バイト目、リトルエンディアン）
            MaxCreateSize = AppCommon.ToInt32(response, 3, false);

            // データサイズを設定
            AlreadySent = 0;
            if (ObjectType == NRFDfuConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                RemainingToSend = ToolDFUImageRef.NRF52AppDatSize;
            } else {
                RemainingToSend = ToolDFUImageRef.NRF52AppBinSize;
            }

            // チェックサムを初期化
            toolDFUUtil = new ToolDFUUtil(MTU);
            toolDFUUtil.DFUObjectChecksumReset();

            // データ分割送信開始
            SendCreateObjectRequest();
        }

        //
        // データ分割送信処理
        //
        private void SendCreateObjectRequest()
        {
            // 送信すべきデータがない場合は終了
            if (RemainingToSend < 1) {
                if (ObjectType == NRFDfuConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                    // DATイメージ転送完了
                    AppCommon.OutputLogDebug("ToolDFU: update init command object done");

                    // BINイメージ転送処理の開始
                    // １回あたりの送信データ最大長を取得
                    SendSelectObjectRequest(NRFDfuConst.NRF_DFU_BYTE_OBJ_DATA);

                } else if (ObjectType == NRFDfuConst.NRF_DFU_BYTE_OBJ_DATA) {
                    // BINイメージ転送完了
                    AppCommon.OutputLogDebug("ToolDFU: update data object done");

                    // これは仮の処理です。
                    TerminateDFUProcess(true);
                }
                return;
            }

            // 送信サイズを通知
            SizeToSend = (MaxCreateSize < RemainingToSend) ? MaxCreateSize : RemainingToSend;

            // CREATE OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_OBJECT_CREATE, ObjectType, 0x00, 0x00, 0x00, 0x00,
                NRFDfuConst.NRF_DFU_BYTE_EOM};
            int offset = 2;
            AppCommon.ConvertUint32ToLEBytes((UInt32)SizeToSend, b, offset);

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveCreateObjectResponse(byte[] response)
        {
            // オブジェクト種別に対応するデータ／サイズを設定
            if (ObjectType == NRFDfuConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                toolDFUUtil.DFUObjectFrameInit(ToolDFUImageRef.NRF52AppDat, SizeToSend, AlreadySent);
            } else {
                toolDFUUtil.DFUObjectFrameInit(ToolDFUImageRef.NRF52AppBin, SizeToSend, AlreadySent);
            }

            // データを送信
            SendWriteCommandObjectRequest();
        }

        private void SendWriteCommandObjectRequest()
        {
            // 送信フレームを生成
            while (toolDFUUtil.DFUObjectFramePrepare(ObjectType)) {
                // DFUリクエスト（生成したフレーム）を送信
                if (DFUDeviceRef.SendDFURequest(toolDFUUtil.GetPreparedFrame()) == false) {
                    TerminateDFUProcess(false);
                    return;
                }
            }

            // 送信済みサイズを更新
            AlreadySent += SizeToSend;

            // 送信データのチェックサム検証に移る
            SendGetCrcRequest();
        }

        private void SendGetCrcRequest()
        {
            // CRC GETコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_CRC_GET, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveGetCrcResponse(byte[] response)
        {
            // レスポンスデータから、エスケープシーケンスを取り除く
            byte[] respUnesc = ToolDFUUtil.UnescapeResponseData(response);

            // レスポンスからデータ長を取得（4〜7バイト目、リトルエンディアン）
            int recvSize = AppCommon.ToInt32(respUnesc, 3, false);

            // 送信データ長を検証
            if (recvSize != AlreadySent) {
                AppCommon.OutputLogError(string.Format(
                    "ToolDFUCommand: send object {0} failed (expected {1} bytes, recv {2} bytes)",
                    ObjectType, AlreadySent, recvSize));
                TerminateDFUProcess(false);
                return;
            }

            // レスポンスからチェックサムを取得（8〜11バイト目、リトルエンディアン）
            UInt32 checksum = (UInt32)AppCommon.ToInt32(respUnesc, 7, false);

            // チェックサムを検証
            if (checksum != toolDFUUtil.DFUObjectChecksumGet()) {
                AppCommon.OutputLogError(string.Format(
                    "ToolDFUCommand: send object {0} failed (checksum error)",
                    ObjectType));
                TerminateDFUProcess(false);
                return;
            }

            // 送信データのチェックサム検証に移る
            SendExecuteObjectRequest();
        }

        private void SendExecuteObjectRequest()
        {
            // EXECUTE OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_OBJECT_EXECUTE, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveExecuteObjectResponse(byte[] response)
        {
            // 未送信サイズを更新
            RemainingToSend -= SizeToSend;

            // 次ブロックの送信処理に移る
            SendCreateObjectRequest();
        }

        //
        // バージョン照会処理
        //
        private ToolDFU ToolDFURef;
        public void SendFWVersionGetRequest(ToolDFU toolDFURef, byte firmwareId)
        {
            // 呼出し元の参照を保持
            ToolDFURef = toolDFURef;

            // SET RECEIPTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_FIRMWARE_VERSION, firmwareId, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUDeviceRef.SendDFURequest(b) == false) {
                ToolDFURef.SoftDeviceVersionResponseReceived(false, 0);
            }
        }

        private void OnFWVersionGetResponse(bool success, byte[] response)
        {
            // レスポンスがNGの場合
            if (success == false || AssertDFUResponseSuccess(response) == false) {
                ToolDFURef.SoftDeviceVersionResponseReceived(false, 0);
            } else {
                // レスポンスからバージョンを取得（5〜8バイト目、リトルエンディアン）
                int versionNumber = AppCommon.ToInt32(response, 4, false);
                ToolDFURef.SoftDeviceVersionResponseReceived(true, versionNumber);
            }
        }

        //
        // DFUレスポンス受信時の処理
        //
        public void OnReceiveDFUResponse(bool success, byte[] response)
        {
            // コマンドバイト（レスポンスの２バイト目）を取得
            byte cmd = response[1];
            if (cmd == NRFDfuConst.NRF_DFU_OP_FIRMWARE_VERSION) {
                // バージョン照会の場合は別関数で処理
                OnFWVersionGetResponse(success, response);
                return;
            }

            // 失敗時はメイン画面に制御を戻す
            if (success == false) {
                TerminateDFUProcess(false);
                return;
            }

            // レスポンスがNGの場合は処理終了
            if (AssertDFUResponseSuccess(response) == false) {
                TerminateDFUProcess(false);
            }

            // コマンドバイトで処理分岐
            switch (cmd) {
            case NRFDfuConst.NRF_DFU_OP_RECEIPT_NOTIF_SET:
                ReceiveSetReceiptRequest(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_MTU_GET:
                ReceiveGetMtuRequest(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_OBJECT_SELECT:
                ReceiveSelectObjectRequest(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_OBJECT_CREATE:
                ReceiveCreateObjectResponse(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_CRC_GET:
                ReceiveGetCrcResponse(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_OBJECT_EXECUTE:
                ReceiveExecuteObjectResponse(response);
                break;
            default:
                break;
            }
        }

        private bool AssertDFUResponseSuccess(byte[] response)
        {
            // レスポンスを検証
            if (response == null || response.Length == 0) {
                return false;
            }

            // ステータスコードを参照し、処理が成功したかどうかを判定
            return (response[2] == NRFDfuConst.NRF_DFU_BYTE_RESP_SUCCESS);
        }
    }
}
