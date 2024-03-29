﻿using MaintenanceToolApp;
using MaintenanceToolApp.CommonProcess;
using System;
using System.Linq;

namespace ToolAppCommon
{
    internal static class BLEProcessConst
    {
        // BLEフレームに関する定義
        public const int INIT_HEADER_LEN = 3;
        public const int CONT_HEADER_LEN = 1;
        public const int BLE_FRAME_LEN = 64;
    }

    public class BLEProcess
    {
        // このクラスのインスタンス
        private static readonly BLEProcess Instance = new BLEProcess();

        // BLEメッセージ受信時のイベント
        public delegate void HandlerOnReceivedResponse(byte CMD, byte[] responseData, bool success, string errorMessage);
        private event HandlerOnReceivedResponse OnReceivedResponse = null!;

        // BLEデバイス接続／切断検知時のイベント
        public delegate void HandlerNotifyConnectionStatus(bool connected);
        private event HandlerNotifyConnectionStatus NotifyConnectionStatus = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnReceivedResponse(HandlerOnReceivedResponse handler)
        {
            Instance.OnReceivedResponse += handler;
        }

        public static void RegisterHandlerNotifyConnectionStatus(HandlerNotifyConnectionStatus handler)
        {
            Instance.NotifyConnectionStatus += handler;
        }

        public static void DoRequestCommand(byte CMD, byte[] data)
        {
            Instance.SendBLEMessage(CMD, data);
        }

        public static void DoConnectCommand()
        {
            // BLE接続試行
            Instance.DoConnectWithFIDOPeripheral(true);
        }

        public static void DisconnctBLE()
        {
            // 接続破棄
            Instance.DisconnectBLE();
        }

        public static string ConnectedDeviceName()
        {
            return Instance.BleService.ConnectedDeviceName();
        }

        //
        // 内部処理
        //
        // BLEデバイス
        private BLEService BleService = new BLEService();

        private BLEProcess()
        {
            // BLEデバイスのイベントを登録
            BleService.OnDataReceived += new BLEService.HandlerOnDataReceived(OnDataReceived);
            BleService.OnTransactionFailed += new BLEService.HandlerOnTransactionFailed(OnTransactionFailed);
            BleService.OnConnectionStatusChanged += new BLEService.HandlerOnConnectionStatusChanged(OnConnectionStatusChanged);

            // 変数初期化
            CMDToSend = 0x00;
            MessageToSend = Array.Empty<byte>();
        }

        // 当初送信コマンドを保持
        private byte CMDToSend { get; set; }
        private byte[] MessageToSend { get; set; }

        private void SendBLEMessage(byte CMD, byte[] message)
        {
            // 送信コマンドを保持
            CMDToSend = CMD;
            MessageToSend = message;

            // メッセージがない場合は終了
            if (message == null || message.Length == 0) {
                OnReceivedResponse(CMD, Array.Empty<byte>(), false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 接続済みの場合は、BLEデバイスにコマンド／メッセージ送信
            if (BleService.IsConnected()) {
                ResumeSendBLEMessage(CMD, message);
                return;
            }

            // 未接続の場合は、接続先のFIDO認証器をスキャン
            DoConnectWithFIDOPeripheral(false);
        }

        private void DoConnectWithFIDOPeripheral(bool connectOnly)
        {
            // 接続先のFIDO認証器をスキャン
            ScanBLEPeripheralParameter parameter = ScanBLEPeripheralParameter.PrepareParameterForFIDO();
            parameter.ConnectOnly = connectOnly;
            new ScanBLEPeripheralProcess().DoProcess(parameter, OnFIDOPeripheralFound);
        }

        private async void OnFIDOPeripheralFound(bool found, string errorMessage, ScanBLEPeripheralParameter parameter)
        {
            if (parameter.BLEPeripheralFound == false) {
                // 接続失敗の旨を通知（エラーログは上位クラスで出力させるようにする）
                OnReceivedResponse(CMDToSend, Array.Empty<byte>(), false, errorMessage);
                return;
            }

            if (parameter.FIDOServiceDataFieldFound) {
                // ペアリングモードでは業務リクエストを実行できない旨のエラーを設定
                OnReceivedResponse(CMDToSend, Array.Empty<byte>(), false, AppCommon.MSG_OCCUR_PAIRINGMODE_ERROR);
                return;
            }

            // サービスに接続
            if (await BleService.StartCommunicate(parameter)) {
                AppLogUtil.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_CONNECTED);
            }

            if (BleService.IsConnected()) {
                // 接続成功の場合
                if (parameter.ConnectOnly) {
                    // 接続成功の旨を通知
                    OnReceivedResponse(0x00, Array.Empty<byte>(), true, AppCommon.MSG_NONE);

                } else {
                    // BLEデバイスにコマンド／メッセージ送信
                    ResumeSendBLEMessage(CMDToSend, MessageToSend);
                }

            } else {
                // 接続失敗の旨を通知（エラーログは上位クラスで出力させるようにする）
                OnReceivedResponse(CMDToSend, Array.Empty<byte>(), false, AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
            }
        }

        private void ResumeSendBLEMessage(byte CMD, byte[] message)
        {
            // BLEデバイスにメッセージをフレーム分割して送信
            SendBLEMessageFrames(CMD, message);

            // リクエスト送信完了メッセージを出力
            AppLogUtil.OutputLogInfo(AppCommon.MSG_REQUEST_SENT);
        }

        private void SendBLEMessageFrames(byte CMD, byte[] message)
        {
            // APDUの長さを取得
            int transferMessageLen = message.Length;

            // 
            // 送信データをフレーム分割
            // 
            //  INITフレーム
            //  1     バイト目: コマンド
            //  2 - 3 バイト目: データ長
            //  残りのバイト  : データ部（64 - 3 = 61）
            //
            //  CONTフレーム
            //  1     バイト目: シーケンス
            //  残りのバイト  : データ部（64 - 1 = 63）
            // 
            byte[] frameData = new byte[BLEProcessConst.BLE_FRAME_LEN];
            int frameLen = 0;
            int transferred = 0;
            int seq = 0;
            while (transferred < transferMessageLen) {
                for (int j = 0; j < frameData.Length; j++) {
                    // フレームデータを初期化
                    frameData[j] = 0;
                }

                if (transferred == 0) {
                    // INITフレーム
                    // ヘッダーをコピー
                    frameData[0] = CMD;
                    frameData[1] = (byte)(transferMessageLen / 256);
                    frameData[2] = (byte)(transferMessageLen % 256);

                    // データをコピー
                    int maxLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.INIT_HEADER_LEN;
                    int dataLenInFrame = (transferMessageLen < maxLen) ? transferMessageLen : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[BLEProcessConst.INIT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = BLEProcessConst.INIT_HEADER_LEN + dataLenInFrame;

                    string dump = AppLogUtil.DumpMessage(frameData, frameLen);
                    AppLogUtil.OutputLogDebug(string.Format("BLE Sent INIT frame: data size={0} length={1}\r\n{2}",
                        transferMessageLen, dataLenInFrame, dump));

                } else {
                    // CONTフレーム
                    // ヘッダーをコピー
                    frameData[0] = (byte)seq;

                    // データをコピー
                    int remaining = transferMessageLen - transferred;
                    int maxLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.CONT_HEADER_LEN;
                    int dataLenInFrame = (remaining < maxLen) ? remaining : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[BLEProcessConst.CONT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = BLEProcessConst.CONT_HEADER_LEN + dataLenInFrame;

                    string dump = AppLogUtil.DumpMessage(frameData, frameLen);
                    AppLogUtil.OutputLogDebug(string.Format("BLE Sent CONT frame: data seq={0} length={1}\r\n{2}",
                        seq++, dataLenInFrame, dump));
                }

                // BLEデバイスにフレームを送信
                BleService.Send(frameData.Take(frameLen).ToArray());
            }
        }

        // 受信データを保持
        private byte[] ReceivedMessage = Array.Empty<byte>();
        private int ReceivedMessageLen = 0;
        private int ReceivedSize = 0;

        private void OnDataReceived(byte[] message)
        {
            // メッセージがない場合は終了
            if (message == null || message.Length == 0) {
                return;
            }

            // 
            // 受信したデータをバッファにコピー
            // 
            //  INITフレーム
            //  1     バイト目: コマンド
            //  2 - 3 バイト目: データ長
            //  残りのバイト  : データ部（64 - 3 = 61）
            //
            //  CONTフレーム
            //  1     バイト目: シーケンス
            //  残りのバイト  : データ部（64 - 1 = 63）
            // 
            int bleInitDataLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.INIT_HEADER_LEN;
            int bleContDataLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.CONT_HEADER_LEN;
            byte cmd = message[0];
            if (cmd > 127) {
                // INITフレームであると判断
                byte cnth = message[1];
                byte cntl = message[2];
                ReceivedMessageLen = cnth * 256 + cntl;
                ReceivedMessage = new byte[BLEProcessConst.INIT_HEADER_LEN + ReceivedMessageLen];
                ReceivedSize = 0;

                // ヘッダーをコピー
                for (int i = 0; i < BLEProcessConst.INIT_HEADER_LEN; i++) {
                    ReceivedMessage[i] = message[i];
                }

                // データをコピー
                int dataLenInFrame = (ReceivedMessageLen < bleInitDataLen) ? ReceivedMessageLen : bleInitDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[BLEProcessConst.INIT_HEADER_LEN + ReceivedSize++] = message[BLEProcessConst.INIT_HEADER_LEN + i];
                }

                if (ReceivedMessage[0] != 0x82) {
                    // キープアライブ以外の場合はログを出力
                    string dump = AppLogUtil.DumpMessage(message, message.Length);
                    AppLogUtil.OutputLogDebug(string.Format(
                        "BLE Recv INIT frame: data size={0} length={1}\r\n{2}",
                        ReceivedMessageLen, dataLenInFrame, dump));
                }

            } else {
                // CONTフレームであると判断
                int seq = message[0];

                // データをコピー
                int remaining = ReceivedMessageLen - ReceivedSize;
                int dataLenInFrame = (remaining < bleContDataLen) ? remaining : bleContDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[BLEProcessConst.INIT_HEADER_LEN + ReceivedSize++] = message[BLEProcessConst.CONT_HEADER_LEN + i];
                }

                string dump = AppLogUtil.DumpMessage(message, message.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "BLE Recv CONT frame: seq={0} length={1}\r\n{2}",
                    seq, dataLenInFrame, dump));
            }

            // 全フレームがそろった場合
            if (ReceivedSize == ReceivedMessageLen) {
                // CMDを抽出
                byte CMD = ReceivedMessage[0];
                if (CMD == 0x82) {
                    // キープアライブの場合は引き続き次のレスポンスを待つ
                    return;
                }

                // 受信データを転送
                AppLogUtil.OutputLogInfo(AppCommon.MSG_RESPONSE_RECEIVED);
                if (ReceivedMessageLen == 0) {
                    OnReceivedResponse(CMD, Array.Empty<byte>(), true, AppCommon.MSG_NONE);

                } else {
                    byte[] response = ReceivedMessage.Skip(BLEProcessConst.INIT_HEADER_LEN).ToArray();
                    OnReceivedResponse(CMD, response, true, AppCommon.MSG_NONE);
                }
            }
        }

        private void OnTransactionFailed(string errorMessage)
        {
            // 送信失敗時はBLEデバイスを切断
            DisconnectBLE();
            OnReceivedResponse(CMDToSend, Array.Empty<byte>(), false, errorMessage);
        }

        private void DisconnectBLE()
        {
            if (BleService.IsConnected()) {
                // 接続ずみの場合はBLEデバイスを切断
                BleService.Disconnect();
            }
        }

        private void OnConnectionStatusChanged(bool connected)
        {
            // BLE接続／切断検知を上位クラスに転送
            NotifyConnectionStatus(connected);
        }
    }
}
