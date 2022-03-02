using MaintenanceToolCommon;
using System;
using System.Linq;

namespace MaintenanceToolGUI
{
    class CCIDProcess
    {
        // CCIDデバイスの参照を保持
        private CCIDDevice Device = null;

        // CCIDデバイスから取得したデータ、ステータスワードを保持
        public byte[] ResponseData = null;
        public uint ResponseSW = 0x00;

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

        public bool SendIns(byte sendIns, byte sendP1, byte sendP2, byte[] sendData, byte sendLe)
        {
            // 例外回避
            if (Device == null) {
                return false;
            }

            // リクエスト送信-->レスポンス受信
            int sizeAlreadySent = 0;
            int sizeToSend = sendData.Length;
            byte sendCla;
            ResponseData = new byte[0];
            ResponseSW = 0;

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
                    return false;
                }

                // 受信データがある場合は連結
                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                ResponseSW = AppCommon.ToUInt16(received, responseDataSize, true);
                if (responseDataSize > 0) {
                    ResponseData.Concat(received.Take(responseDataSize));
                }
                AppCommon.OutputLogDebug(string.Format("CCID response: Data({0} bytes) SW(0x{1,0:x4})", responseDataSize, ResponseSW));

                // 送信済みサイズを更新
                sizeAlreadySent += thisSendSize;

            } while (sizeAlreadySent < sizeToSend);

            while (ResponseSW >> 8 == 0x61) {
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
                    return false;
                }

                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                ResponseSW = AppCommon.ToUInt16(received, responseDataSize, true);
                if (ResponseSW != 0x9000 && ResponseSW >> 8 != 0x61) {
                    // ステータスワードが不正の場合は制御を戻す
                    return false;
                }

                // 受信データがある場合は連結
                if (responseDataSize > 0) {
                    ResponseData.Concat(received.Take(responseDataSize));
                }
            }

            return true;
        }
    }
}
