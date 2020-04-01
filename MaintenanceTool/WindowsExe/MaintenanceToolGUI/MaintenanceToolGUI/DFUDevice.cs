using MaintenanceToolCommon;
using System;
using System.IO.Ports;
using System.Threading;

namespace MaintenanceToolGUI
{
    class DFUDevice
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        // 仮想COMポート
        private SerialPort SerialPortRef = null;
        private string SerialPortName = "";

        public DFUDevice(ToolDFU td)
        {
            ToolDFURef = td;
        }

        // DFU接続完了時のイベント
        public delegate void DFUConnectionEstablishedEventHandler(bool success);
        public event DFUConnectionEstablishedEventHandler DFUConnectionEstablishedEvent;

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
                SerialPortRef = new SerialPort(port);
                if (SerialPortRef == null) {
                    AppCommon.OutputLogError(string.Format(
                        "DFUDevice.OpenDFUDevice({0}): SerialPortRef is null", port));
                    return false;
                }

                SerialPortRef.DataReceived += SerialPortDataReceived;
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
                SerialPortRef.DataReceived -= SerialPortDataReceived;
                SerialPortRef.Close();
                AppCommon.OutputLogDebug("DFUDevice.CloseDFUDevice: SerialPort is closed");

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("DFUDevice.CloseDFUDevice: {0}", e.Message));

            } finally {
                SerialPortRef = null;
            }
        }

        private void SerialPortDataReceived(object sender, SerialDataReceivedEventArgs a)
        {
        }
    }
}
