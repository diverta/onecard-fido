using System;
using System.IO.Ports;
using System.Threading;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    internal static class USBDFUConst
    {
        // DFU関連の定義
        public const int NRF_DFU_BYTE_EOM = 0xc0;
        public const int NRF_DFU_BYTE_RESP_START = 0x60;
        public const int NRF_DFU_BYTE_RESP_SUCCESS = 0x01;

        // DFUオブジェクト種別
        public const int NRF_DFU_BYTE_OBJ_INIT_CMD = 0x01;
        public const int NRF_DFU_BYTE_OBJ_DATA = 0x02;

        // DFUコマンド種別（nRF52用）
        public const int NRF_DFU_OP_OBJECT_CREATE = 0x01;
        public const int NRF_DFU_OP_RECEIPT_NOTIF_SET = 0x02;
        public const int NRF_DFU_OP_CRC_GET = 0x03;
        public const int NRF_DFU_OP_OBJECT_EXECUTE = 0x04;
        public const int NRF_DFU_OP_OBJECT_SELECT = 0x06;
        public const int NRF_DFU_OP_MTU_GET = 0x07;
        public const int NRF_DFU_OP_OBJECT_WRITE = 0x08;
        public const int NRF_DFU_OP_PING = 0x09;
        public const int NRF_DFU_OP_FIRMWARE_VERSION = 0x0b;
    }

    internal class USBDFUService
    {
        //
        // CDC ACM送受信関連イベント
        //
        public delegate void HandlerOnConnectedToDFUService(bool success);
        private event HandlerOnConnectedToDFUService OnConnectedToDFUService = null!;

        public delegate void HandlerOnReceivedDFUResponse(bool success, byte[] responseData);
        private event HandlerOnReceivedDFUResponse OnReceivedResponse = null!;

        //
        // 接続処理
        //
        // コールバック関数を保持
        private HandlerOnConnectedToDFUService HandlerRef = null!;

        // 仮想COMポート名の検索回数
        private int ResumeRemaining;

        // 仮想COMポートリストの参照を保持
        private string[] COMPortNameList = null!;

        // 仮想COMポートリストの現在探索インデックス
        private int COMPortIndex;

        // 仮想COMポート
        private SerialPort SerialPortRef = null!;
        private string SerialPortName = "";

        public void ConnectUSBDFUService(HandlerOnConnectedToDFUService handler)
        {
            // イベントを登録
            HandlerRef = handler;
            OnConnectedToDFUService += HandlerRef;

            // 最大10回繰り返す
            ResumeRemaining = 10;

            // COMポート検索処理を続行
            ResumeConnectUSBDFUService();

        }

        private void ResumeConnectUSBDFUService()
        {
            if (ResumeRemaining == 0) {
                // 最大繰り返し回数に達した場合は終了
                AppLogUtil.OutputLogError("USBDFUService.ConnectUSBDFUService: DFU target device not found");
                FuncOnConnectedToDFUService(false);
                return;
            }

            // 0.5秒間ウェイト
            Thread.Sleep(500);

            // COMポートリストを取得
            if (GetCOMPortNameList() == false) {
                // COMポートリストが空の場合はリトライ
                ResumeRemaining--;
                AppLogUtil.OutputLogDebug(string.Format("USBDFUService.ConnectUSBDFUService retry: remaining {0} times", ResumeRemaining));
                ResumeConnectUSBDFUService();
                return;
            }

            // １番目のCOMポートの処理
            COMPortIndex = 0;
            SearchCOMPort();
        }

        private bool GetCOMPortNameList()
        {
            string[] PortList = SerialPort.GetPortNames();
            if (PortList.Length == 0) {
                // 接続されているデバイスがない場合
                COMPortNameList = null!;
                return false;
            }

            // リストの参照を保持
            COMPortNameList = PortList;
            return true;
        }

        private void SearchCOMPort()
        {
            if (COMPortIndex == COMPortNameList.Length) {
                // リストの最終端に達した場合は終了
                return;
            }

            // COMポート名を取得
            string port = COMPortNameList[COMPortIndex];

            // 接続を実行
            if (OpenDFUDevice(port) == false) {
                // 接続できない場合は次のCOMポートの処理に移る
                COMPortIndex++;
                SearchCOMPort();
                return;
            }

            // 接続されているポート名を退避
            SerialPortName = port;

            // DFU対象デバイスに接続された場合はDFU PINGを実行
            if (SendPingRequest() == false) {
                // 接続を閉じる
                CloseDFUDevice();

                // 送信できない場合は次のCOMポートの処理に移る
                COMPortIndex++;
                SearchCOMPort();
                return;
            }
        }

        private void FuncOnConnectedToDFUService(bool success)
        {
            // FIDO認証器に接続完了
            OnConnectedToDFUService(success);
            OnConnectedToDFUService -= HandlerRef;
        }

        //
        // DFUデバイスの接続／切断処理
        //
        // イベントハンドラーを保持
        private SerialDataReceivedEventHandler SerialDataReceivedEventHandlerRef = null!;

        private bool OpenDFUDevice(string port)
        {
            try {
                // ポート初期化
                SerialPortRef = new SerialPort(port);
                if (SerialPortRef == null) {
                    AppLogUtil.OutputLogError(string.Format("USBDFUService.OpenDFUDevice({0}): SerialPortRef is null", port));
                    return false;
                }
                SerialPortRef.WriteTimeout = 1000;
                SerialPortRef.ReadTimeout = 1000;
                SerialPortRef.BaudRate = 9600;

                // ポートを開く
                SerialPortRef.Open();
                if (SerialPortRef.IsOpen) {
                    // イベントを登録
                    SerialDataReceivedEventHandlerRef = new SerialDataReceivedEventHandler(SerialPortDataReceived);
                    SerialPortRef.DataReceived += SerialDataReceivedEventHandlerRef;

                    // ポートオープン完了
                    AppLogUtil.OutputLogDebug(string.Format("USBDFUService.OpenDFUDevice({0}): SerialPort is opened", port));
                    return true;
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("USBDFUService.OpenDFUDevice({0}): {1}", port, e.Message));
            }
            return false;
        }

        public void CloseDFUDevice()
        {
            if (SerialPortRef == null) {
                AppLogUtil.OutputLogDebug("USBDFUService.CloseDFUDevice: SerialPortRef is null");
                return;
            }

            try {
                if (SerialPortRef.IsOpen) {
                    // イベントの解除
                    SerialPortRef.DataReceived -= SerialDataReceivedEventHandlerRef;

                    // ポートを閉じる
                    SerialPortRef.Close();
                    AppLogUtil.OutputLogDebug("USBDFUService.CloseDFUDevice: SerialPort is closed");
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogDebug(string.Format("USBDFUService.CloseDFUDevice: {0}", e.Message));

            } finally {
                SerialPortRef = null!;
            }
        }

        //
        // DFUリクエスト／レスポンス処理
        //
        public void RegisterHandlerOnReceivedResponse(HandlerOnReceivedDFUResponse handler)
        {
            OnReceivedResponse += handler;
        }

        public void UnregisterHandlerOnReceivedResponse(HandlerOnReceivedDFUResponse handler)
        {
            OnReceivedResponse -= handler;
        }

        public bool SendDFURequest(byte[] b)
        {
            try {
                // DFUリクエストを送信
                SerialPortRef.Write(b, 0, b.Length);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("USBDFUService.SendDFURequest: {0}", e.Message));
                return false;
            }
        }

        public bool DTROperation()
        {
            try {
                // DTRをOnに変更
                SerialPortRef.DtrEnable = true;
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("USBDFUService.DTROperation: {0}", e.Message));
                return false;
            }
        }

        private void SerialPortDataReceived(object sender, SerialDataReceivedEventArgs a)
        {
            // EOF受信時は何もしない
            if (a.EventType.Equals(SerialData.Eof)) {
                AppLogUtil.OutputLogDebug("USBDFUService.SerialPortDataReceived: SerialData is EOF");
                return;
            }

            // シリアルポートをオープンしていない場合は何もしない
            if (SerialPortRef == null) {
                AppLogUtil.OutputLogDebug("USBDFUService.SerialPortDataReceived: SerialPortRef is null");
                return;
            }
            if (SerialPortRef.IsOpen == false) {
                AppLogUtil.OutputLogDebug("USBDFUService.SerialPortDataReceived: SerialPortRef is not opened");
                return;
            }

            // DFUレスポンスデータを読込
            bool success;
            Byte[] response = null!;
            try {
                int bytesToRead = SerialPortRef.BytesToRead;
                if (bytesToRead == 0) {
                    AppLogUtil.OutputLogDebug("USBDFUService.SerialPortDataReceived: SerialPortRef.BytesToRead is zero");
                    success = false;

                } else {
                    response = new Byte[bytesToRead];
                    SerialPortRef.Read(response, 0, bytesToRead);
                    success = true;
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("USBDFUService.serialPortDataReceived: {0}", e.Message));
                success = false;
            }

            // DFUレスポンス受信時の処理を実行
            FuncOnDFUResponseReceived(success, response);
        }

        private void FuncOnDFUResponseReceived(bool success, byte[] response)
        {
            // レスポンスの２バイト目（コマンドバイト）で処理分岐
            byte cmd = response[1];
            if (cmd == USBDFUConst.NRF_DFU_OP_PING) {
                // DFU PING応答時の処理
                ReceivePingRequest(success, response);

            } else {
                // DFU PING以外の応答時は、外部に通知
                OnReceivedResponse(success, response);
            }
        }

        //
        // DFU PING処理
        //
        private bool SendPingRequest()
        {
            // PINGコマンドを生成（DFUリクエスト）
            byte id = 0xac;
            byte[] b = new byte[] { USBDFUConst.NRF_DFU_OP_PING, id, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (SendDFURequest(b)) {
                return DTROperation();
            } else {
                return false;
            }
        }

        private void ReceivePingRequest(bool success, byte[] response)
        {
            if (success == false) {
                // 接続を閉じる
                CloseDFUDevice();

                // PING失敗の場合は次のCOMポートの処理に移る
                COMPortIndex++;
                SearchCOMPort();
                return;
            }

            // PING成功の場合は検索処理を終了
            AppLogUtil.OutputLogDebug(string.Format("USBDFUService: DFU target device found on {0}", SerialPortName));
            FuncOnConnectedToDFUService(true);
        }
    }
}
