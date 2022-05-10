using System;
using System.IO.Ports;
using System.Threading;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    internal static class NRFDfuConst
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

    class DFUDevice
    {
        // 仮想COMポート
        private SerialPort SerialPortRef = null;
        private string SerialPortName = "";

        public DFUDevice()
        {
            // イベントの登録
            DFUResponseReceivedEvent += new DFUResponseReceivedEventHandler(OnDFUResponseReceived);
        }

        // DFU接続完了時のイベント
        public delegate void DFUConnectionEstablishedEventHandler(bool success);
        public event DFUConnectionEstablishedEventHandler DFUConnectionEstablishedEvent;

        // DFU応答時のイベント
        private delegate void DFUResponseReceivedEventHandler(bool success, byte[] response);
        private event DFUResponseReceivedEventHandler DFUResponseReceivedEvent;

        // DFU応答時の外部通知イベント
        public delegate void NotifyDFUResponseEventHandler(bool success, byte[] response);
        public event NotifyDFUResponseEventHandler NotifyDFUResponseEvent;

        // 仮想COMポート名の検索回数
        private int ResumeRemaining;

        // 仮想COMポートリストの参照を保持
        private string[] COMPortNameList;

        // 仮想COMポートリストの現在探索インデックス
        private int COMPortIndex;

        public void SearchACMDevicePath()
        {
            // 最大10回繰り返す
            ResumeRemaining = 10;

            // COMポート検索処理を続行
            ResumeSearchACMDevicePath();
        }

        private void ResumeSearchACMDevicePath()
        {
            if (ResumeRemaining == 0) {
                // 最大繰り返し回数に達した場合は終了
                AppUtil.OutputLogError("DFUDevice.SearchACMDevicePath: DFU target device not found");
                DFUConnectionEstablishedEvent(false);
                return;
            }

            // 0.5秒間ウェイト
            Thread.Sleep(500);

            // COMポートリストを取得
            if (GetCOMPortNameList() == false) {
                // COMポートリストが空の場合はリトライ
                ResumeRemaining--;
                AppUtil.OutputLogDebug(string.Format(
                    "DFUDevice.SearchACMDevicePath retry: remaining {0} times", ResumeRemaining));
                ResumeSearchACMDevicePath();
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
                COMPortNameList = null;
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

        private bool SendPingRequest()
        {
            // PINGコマンドを生成（DFUリクエスト）
            byte id = 0xac;
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_PING, id, NRFDfuConst.NRF_DFU_BYTE_EOM };

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

            // PING成功の場合は検索処理を終了（接続はキープする）
            AppUtil.OutputLogDebug(string.Format(
                "DFUDevice: DFU target device found on {0}",
                SerialPortName));
            DFUConnectionEstablishedEvent(true);
        }

        //
        // DFUデバイスの接続／切断処理
        //
        private bool OpenDFUDevice(string port)
        {
            try {
                // ポート初期化
                SerialPortRef = new SerialPort(port);
                if (SerialPortRef == null) {
                    AppUtil.OutputLogError(string.Format(
                        "DFUDevice.OpenDFUDevice({0}): SerialPortRef is null", port));
                    return false;
                }
                SerialPortRef.WriteTimeout = 1000;
                SerialPortRef.ReadTimeout = 1000;
                SerialPortRef.BaudRate = 9600;

                // ポートを開く
                SerialPortRef.DataReceived += new SerialDataReceivedEventHandler(SerialPortDataReceived);
                SerialPortRef.Open();
                if (SerialPortRef.IsOpen) {
                    AppUtil.OutputLogDebug(string.Format(
                        "DFUDevice.OpenDFUDevice({0}): SerialPort is opened", port));
                    return true;
                }

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("DFUDevice.OpenDFUDevice({0}): {1}", port, e.Message));
            }
            return false;
        }

        public void CloseDFUDevice()
        {
            if (SerialPortRef == null) {
                AppUtil.OutputLogDebug("DFUDevice.CloseDFUDevice: SerialPortRef is null");
                return;
            }

            try {
                if (SerialPortRef.IsOpen) {
                    SerialPortRef.Close();
                    AppUtil.OutputLogDebug("DFUDevice.CloseDFUDevice: SerialPort is closed");
                }

            } catch (Exception e) {
                AppUtil.OutputLogDebug(string.Format("DFUDevice.CloseDFUDevice: {0}", e.Message));

            } finally {
                SerialPortRef = null;
            }
        }

        //
        // DFUリクエスト／レスポンス処理
        //
        private byte[] DFURequestBytes;
        private byte[] DFUResponseBytes;

        public bool SendDFURequest(byte[] b)
        {
            // 送信リクエストを保持
            DFURequestBytes = b;

            try {
                // DFUリクエストを送信
                SerialPortRef.Write(b, 0, b.Length);
                return true;

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("DFUDevice.SendDFURequest: {0}", e.Message));
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
                AppUtil.OutputLogError(string.Format("DFUDevice.DTROperation: {0}", e.Message));
                return false;
            }
        }

        private void SerialPortDataReceived(object sender, SerialDataReceivedEventArgs a)
        {
            // EOF受信時は何もしない
            if (a.EventType.Equals(SerialData.Eof)) {
                AppUtil.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialData is EOF");
                return;
            }
            // シリアルポートをオープンしていない場合は何もしない
            if (SerialPortRef == null) {
                AppUtil.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialPortRef is null");
                return;
            }
            if (SerialPortRef.IsOpen == false) {
                AppUtil.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialPortRef is not opened");
                return;
            }

            try {
                // DFUレスポンスデータを読込
                int bytesToRead = SerialPortRef.BytesToRead;
                if (bytesToRead == 0) {
                    AppUtil.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialPortRef.BytesToRead is zero");
                    DFUResponseReceivedEvent(false, null);
                    return;
                }

                Byte[] response = new Byte[bytesToRead];
                SerialPortRef.Read(response, 0, bytesToRead);

                // 受信レスポンスを保持
                DFUResponseBytes = response;

                // DFUレスポンス受信時の処理を実行
                DFUResponseReceivedEvent(true, response);

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("DFUDevice.serialPortDataReceived: {0}", e.Message));
                DFUResponseReceivedEvent(false, null);
            }
        }

        private void OnDFUResponseReceived(bool success, byte[] response)
        {
            // レスポンスの２バイト目（コマンドバイト）で処理分岐
            byte cmd = response[1];
            if (cmd == NRFDfuConst.NRF_DFU_OP_PING) {
                // DFU PING応答時の処理
                ReceivePingRequest(success, response);

            } else {
                // DFU PING以外の応答時は、外部に通知
                NotifyDFUResponseEvent(success, response);
            }
        }
    }
}
