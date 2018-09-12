namespace U2FHelper
{
    class BLEProcess
    {
        // BLEデバイス関連
        private BLEService bleService = new BLEService();

        // メイン画面の参照を保持
        private MainForm mainForm;

        // メッセージテキスト送信用のイベント
        public delegate void MessageTextEventHandler(string messageText);
        public event MessageTextEventHandler MessageTextEvent;

        // BLEメッセージ受信時のイベント
        public delegate void ReceiveBLEMessageEventHandler(bool ret, byte[] message, int length);
        public event ReceiveBLEMessageEventHandler ReceiveBLEMessageEvent;

        public BLEProcess()
        {
            // BLEデバイスのイベントを登録
            bleService.DataReceived += new BLEService.dataReceivedEvent(BLEDeviceDataReceived);
        }

        public void SetMainForm(MainForm f)
        {
            // メインフォームの参照を保持
            mainForm = f;
        }

        public async void DoXferMessage(byte[] message, int length)
        {
            // BLEデバイスに接続
            if (await bleService.Connect() == false) {
                MessageTextEvent(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED + "\r\n");
                ReceiveBLEMessageEvent(false, null, 0);
                return;
            }

            //
            // TODO: 送信データの各フレームを生成
            //
            string dump = AppCommon.DumpMessage(message, length);
            AppCommon.OutputLogToFile(string.Format("Sent {0} bytes: \r\n{1}",
                length, dump), true);

            // BLEデバイスにメッセージを送信
            if (await bleService.Send(message) == false) {
                MessageTextEvent(AppCommon.MSG_REQUEST_SEND_FAILED + "\r\n");
                ReceiveBLEMessageEvent(false, null, 0);
                return;
            }
        }

        public void BLEDeviceDataReceived(byte[] message, int length)
        {
            //
            // TODO: 受信した各フレームから受信データを生成
            //
            string dump = AppCommon.DumpMessage(message, length);
            AppCommon.OutputLogToFile(string.Format("Received {0} bytes: \r\n{1}",
                length, dump), true);

            // 全フレームがそろった場合は切断し、
            // 受信データを転送
            bleService.Disconnect();
            ReceiveBLEMessageEvent(true, message, length);
        }
    }
}
