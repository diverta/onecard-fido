using System;
using System.Collections.Generic;
using System.Linq;

namespace ToolAppCommon
{
    internal static class CCIDProcessConst
    {
        public const UInt16 SW_PIN_RETRIES = 0x63C0;
        public const UInt16 SW_UNABLE_TO_PROCESS = 0x6900;
        public const UInt16 SW_SEC_STATUS_NOT_SATISFIED = 0x6982;
        public const UInt16 SW_ERR_AUTH_BLOCKED = 0x6983;
        public const UInt16 SW_SUCCESS = 0x9000;
    }

    public class CCIDParameter
    {
        public byte INS { get; set; }
        public byte P1 { get; set; }
        public byte P2 { get; set; }
        public byte[] Data { get; set; } = Array.Empty<byte>();
        public byte Le { get; set; }
    }

    public class CCIDProcess
    {
        // このクラスのインスタンス
        private static readonly CCIDProcess Instance = new CCIDProcess();

        // CCIDメッセージ受信時のイベント
        public delegate void HandlerOnReceivedResponse(bool success, byte[] responseData, UInt16 responseSW);
        private event HandlerOnReceivedResponse OnReceivedResponse = null!;

        //
        // 外部公開用
        //
        public static bool ConnectCCID()
        {
            return Instance.Connect();
        }

        public static void DoRequestCommand(CCIDParameter parameter, HandlerOnReceivedResponse handler)
        {
            Instance.OnReceivedResponse += handler;
            Instance.SendIns(parameter.INS, parameter.P1, parameter.P2, parameter.Data, parameter.Le);
            Instance.OnReceivedResponse -= handler;
        }

        public static void DisconnectCCID()
        {
            Instance.Disconnect();
        }

        public static string GetReaderName()
        {
            return Instance.ConnectedReaderName;
        }

        //
        // 内部処理
        //
        // CCIDデバイスの参照を保持
        private CCIDDevice Device = null!;

        // 接続中のデバイス名
        private string ConnectedReaderName = String.Empty;

        private bool Connect()
        {
            // デバイスに接続
            Device = new CCIDDevice();
            bool ret = Device.ConnectDevice();

            // 接続中のデバイス名称を保持
            ConnectedReaderName = Device.GetConnectedReaderName();
            return ret;
        }

        private void Disconnect()
        {
            if (Device != null) {
                Device.Disconnect();
            }
            Device = null!;
        }

        private void SendIns(byte sendIns, byte sendP1, byte sendP2, byte[] sendData, byte sendLe)
        {
            // リクエスト送信-->レスポンス受信
            int sizeAlreadySent = 0;
            int sizeToSend = sendData.Length;
            byte sendCla;
            byte[] responseData = Array.Empty<byte>();
            UInt16 responseSW = 0;
            IEnumerable<byte> responseBytes;

            // 例外回避
            if (Device == null || sendData == null) {
                OnReceivedResponse(false, responseData, CCIDProcessConst.SW_UNABLE_TO_PROCESS);
                return;
            }

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
                int frameToSendSize = thisSendSize + 6;
                if (sizeToSend == 0) {
                    frameToSendSize = 5;
                }
                byte[] frameToSend = new byte[frameToSendSize];
                int offset = 0;
                frameToSend[offset++] = sendCla;
                frameToSend[offset++] = sendIns;
                frameToSend[offset++] = sendP1;
                frameToSend[offset++] = sendP2;

                if (sizeToSend > 0) {
                    frameToSend[offset++] = (byte)thisSendSize;
                    Array.Copy(sendData, sizeAlreadySent, frameToSend, offset, thisSendSize);
                    offset += thisSendSize;
                }
                frameToSend[offset] = sendLe;

                if (Device.Transmit(frameToSend) == false) {
                    // 送信エラーが発生した場合
                    OnReceivedResponse(false, responseData, CCIDProcessConst.SW_UNABLE_TO_PROCESS);
                    return;
                }

                // 受信データがある場合は連結
                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                responseSW = AppUtil.ToUInt16(received, responseDataSize, true);
                if (responseDataSize > 0) {
                    responseBytes = responseData.Concat(received.Take(responseDataSize));
                    responseData = responseBytes.ToArray();
                }

                // 送信済みサイズを更新
                sizeAlreadySent += thisSendSize;

            } while (sizeAlreadySent < sizeToSend);

            while (responseSW >> 8 == 0x61) {
                // GET RESPONSE APDU
                int offset = 0;
                byte[] FrameToSend = new byte[5];
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset++] = 0xc0;
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset++] = 0x00;
                FrameToSend[offset] = (byte)(responseSW & 0x00ff);

                if (Device.Transmit(FrameToSend) == false) {
                    // 送信エラーが発生した場合
                    OnReceivedResponse(false, responseData, CCIDProcessConst.SW_UNABLE_TO_PROCESS);
                    return;
                }

                byte[] received = Device.GetReceivedBytes();
                int responseDataSize = received.Length - 2;
                responseSW = AppUtil.ToUInt16(received, responseDataSize, true);
                if (responseSW != CCIDProcessConst.SW_SUCCESS && responseSW >> 8 != 0x61) {
                    // ステータスワードが不正の場合は制御を戻す
                    OnReceivedResponse(false, responseData, responseSW);
                    return;
                }

                // 受信データがある場合は連結
                if (responseDataSize > 0) {
                    responseBytes = responseData.Concat(received.Take(responseDataSize));
                    responseData = responseBytes.ToArray();
                }
            }

            // コマンドに制御を戻す
            OnReceivedResponse(true, responseData, responseSW);
        }
    }
}
