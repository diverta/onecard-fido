using System;
using System.Linq;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class CCIDConst
    {
        public const UInt16 SW_PIN_RETRIES = 0x63C0;
        public const UInt16 SW_UNABLE_TO_PROCESS = 0x6900;
        public const UInt16 SW_SEC_STATUS_NOT_SATISFIED = 0x6982;
        public const UInt16 SW_ERR_AUTH_BLOCKED = 0x6983;
        public const UInt16 SW_SUCCESS = 0x9000;
    }

    class CCIDProcess
    {
        // CCIDデバイスの参照を保持
        private CCIDDevice Device = null;

        // CCID I/Fからデータ受信時のイベント
        public delegate void DataReceivedEvent(byte[] responseData, UInt16 responseSW);
        public event DataReceivedEvent OnDataReceived;

        public CCIDProcess()
        {
        }

        public bool Connect()
        {
            Device = new CCIDDevice();
            return Device.ConnectDevice();
        }

        public void Disconnect()
        {
            if (Device != null) {
                Device.Disconnect();
            }
            Device = null;
        }

        public void SendIns(byte sendIns, byte sendP1, byte sendP2, byte[] sendData, byte sendLe)
        {
            // 例外回避
            if (Device == null || sendData == null) {
                OnDataReceived(null, CCIDConst.SW_UNABLE_TO_PROCESS);
                return;
            }

            // リクエスト送信-->レスポンス受信
            int sizeAlreadySent = 0;
            int sizeToSend = sendData.Length;
            byte sendCla;
            byte[] responseData = new byte[0];
            UInt16 responseSW = 0;

            do {
                // 送信サイズとCLA値を設定
                int thisSendSize = 255;
                if (sizeAlreadySent + thisSendSize < sizeToSend) {
                    // 最終フレームでない場合
                    sendCla = 0x10;
                } else {
                    // 最終フレームの場合
                    thisSendSize = sizeToSend - sizeAlreadySent;
                    sendCla = 0x00;
                }

                // 今回送信分のAPDUデータを抽出し、送信処理を実行
                byte[] frameToSend = new byte[thisSendSize + 6];
                int offset = 0;
                frameToSend[offset++] = sendCla;
                frameToSend[offset++] = sendIns;
                frameToSend[offset++] = sendP1;
                frameToSend[offset++] = sendP2;
                frameToSend[offset++] = (byte)thisSendSize;
                Array.Copy(sendData, sizeAlreadySent, frameToSend, offset, thisSendSize);
                offset += thisSendSize;
                frameToSend[offset] = sendLe;

                if (Device.Transmit(frameToSend) == false) {
                    // 送信エラーが発生した場合
                    OnDataReceived(null, CCIDConst.SW_UNABLE_TO_PROCESS);
                    return;
                }

                // 受信データがある場合は連結
                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                responseSW = AppUtil.ToUInt16(received, responseDataSize, true);
                if (responseDataSize > 0) {
                    responseData.Concat(received.Take(responseDataSize));
                }

                // 送信済みサイズを更新
                sizeAlreadySent += thisSendSize;

            } while (sizeAlreadySent < sizeToSend);

            while (responseSW >> 8 == 0x61) {
                // GET RESPONSE APDU
                int offset = 0;
                byte[] FrameToSend = new byte[6];
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset++] = 0xc0;
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset] = 0xff;

                if (Device.Transmit(FrameToSend) == false) {
                    // 送信エラーが発生した場合
                    OnDataReceived(null, CCIDConst.SW_UNABLE_TO_PROCESS);
                    return;
                }

                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                responseSW = AppUtil.ToUInt16(received, responseDataSize, true);
                if (responseSW != CCIDConst.SW_SUCCESS && responseSW >> 8 != 0x61) {
                    // ステータスワードが不正の場合は制御を戻す
                    OnDataReceived(null, responseSW);
                    return;
                }

                // 受信データがある場合は連結
                if (responseDataSize > 0) {
                    responseData.Concat(received.Take(responseDataSize));
                }
            }

            // コマンドに制御を戻す
            OnDataReceived(responseData, responseSW);
        }
    }
}
