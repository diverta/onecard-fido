using MaintenanceToolCommon;
using System;
using System.IO.Ports;
using System.Threading;

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
    }

    class DFUDevice
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        // 仮想COMポート
        private SerialPort SerialPortRef = null;
        private string SerialPortName = "";

        public DFUDevice(ToolDFU td)
        {
            // DFU処理クラスの参照を保持
            ToolDFURef = td;

            // イベントの登録
            this.DFUResponseReceivedEvent += new DFUResponseReceivedEventHandler(OnDFUResponseReceived);
        }

        // DFU接続完了時のイベント
        public delegate void DFUConnectionEstablishedEventHandler(bool success);
        public event DFUConnectionEstablishedEventHandler DFUConnectionEstablishedEvent;

        // DFU応答時のイベント
        public delegate void DFUResponseReceivedEventHandler(bool success, byte[] response);
        public event DFUResponseReceivedEventHandler DFUResponseReceivedEvent;

        public void SearchACMDevicePath()
        {
            // 最大５秒間繰り返す
            for (int i = 0; i < 10; i++) {
                // 0.5秒間ウェイト
                Thread.Sleep(500);
                // DFU対象デバイスに接続
                if (GetACMDevicePath()) {
                    // DFU対象デバイスに接続された場合
                    // 接続を閉じる
                    CloseDFUDevice();
                    AppCommon.OutputLogDebug(string.Format(
                        "DFUDevice.SearchACMDevicePath: DFU target device found: {0}", 
                        SerialPortName));
                    DFUConnectionEstablishedEvent(true);
                    return;
                }
            }
            AppCommon.OutputLogError("DFUDevice.SearchACMDevicePath: DFU target device not found");
            DFUConnectionEstablishedEvent(false);
        }

        private bool GetACMDevicePath()
        {
            string[] PortList = SerialPort.GetPortNames();
            if (PortList.Length == 0) {
                // 接続されているデバイスがない場合は終了
                AppCommon.OutputLogDebug("DFUDevice.GetDFUDevicePath: SerialPortList is empty");
                SerialPortName = "";
                return false;
            }

            // 接続されているUSB CDC ACMデバイスのパスを走査
            foreach (string port in PortList) {
                AppCommon.OutputLogDebug(string.Format(
                    "DFUDevice.GetDFUDevicePath: SerialPort = {0}", port));
                // 接続を実行
                if (OpenDFUDevice(port)) {
                    SerialPortName = port;
                    return true;
                }
            }
            SerialPortName = "";
            return false;
        }

        private bool OpenDFUDevice(string port)
        {
            try {
                // ポート初期化
                SerialPortRef = new SerialPort(port);
                if (SerialPortRef == null) {
                    AppCommon.OutputLogError(string.Format(
                        "DFUDevice.OpenDFUDevice({0}): SerialPortRef is null", port));
                    return false;
                }
                SerialPortRef.WriteTimeout = 1000;
                SerialPortRef.ReadTimeout = 1000;
                SerialPortRef.BaudRate = 115200;

                // ポートを開く
                SerialPortRef.DataReceived += new SerialDataReceivedEventHandler(SerialPortDataReceived);
                SerialPortRef.Open();
                if (SerialPortRef.IsOpen) {
                    AppCommon.OutputLogDebug(string.Format(
                        "DFUDevice.OpenDFUDevice({0}): SerialPort is opened", port));
                    return true;
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("DFUDevice.OpenDFUDevice({0}): {1}", port, e.Message));
            }
            return false;
        }

        private void CloseDFUDevice()
        {
            if (SerialPortRef == null) {
                AppCommon.OutputLogDebug("DFUDevice.CloseDFUDevice: SerialPortRef is null");
                return;
            }

            try {
                if (SerialPortRef.IsOpen) {
                    SerialPortRef.Close();
                    AppCommon.OutputLogDebug("DFUDevice.CloseDFUDevice: SerialPort is closed");
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("DFUDevice.CloseDFUDevice: {0}", e.Message));

            } finally {
                SerialPortRef = null;
            }
        }

        private void SerialPortDataReceived(object sender, SerialDataReceivedEventArgs a)
        {
            // シリアルポートをオープンしていない場合は何もしない
            if (SerialPortRef == null) {
                AppCommon.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialPortRef is null");
                return;
            }
            if (SerialPortRef.IsOpen == false) {
                AppCommon.OutputLogDebug("DFUDevice.SerialPortDataReceived: SerialPortRef is not opened");
                return;
            }

            try {
                // DFUレスポンスデータを読込
                Byte[] response = new Byte[SerialPortRef.BytesToRead];
                SerialPortRef.Read(response, 0, response.GetLength(0));

                string dump = AppCommon.DumpMessage(response, response.GetLength(0));
                AppCommon.OutputLogDebug(string.Format("DFUDevice.serialPortDataReceived:\r\n{0}", dump));

                // DFUレスポンス受信時の処理を実行
                DFUResponseReceivedEvent(true, response);

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("DFUDevice.serialPortDataReceived: {0}", e.Message));
                DFUResponseReceivedEvent(false, null);
            }
        }

        private void OnDFUResponseReceived(bool success, byte[] response)
        {
        }
    }
}
