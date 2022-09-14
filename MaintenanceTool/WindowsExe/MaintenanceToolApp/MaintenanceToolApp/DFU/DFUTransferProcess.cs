﻿using MaintenanceToolApp.Common;
using System;
using System.Linq;
using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class DFUTransferProcess
    {
        // このクラスのインスタンス
        private static readonly DFUTransferProcess Instance = new DFUTransferProcess();

        // 応答タイムアウト監視用タイマー
        private CommonTimer responseTimer = null!;

        private DFUTransferProcess()
        {
            // 応答タイムアウト発生時のイベントを登録
            responseTimer = new CommonTimer("DFUTransferProcess", 10000);
            responseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;
        }

        // BLE SMPサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly BLESMPService SMPService = new BLESMPService();

        // 上位クラス／パラメーターの参照を保持
        private DFUProcess DFUProcess = null!;
        private DFUParameter Parameter = null!;

        public static void InvokeTransferProcess(DFUProcess process, DFUParameter parameter)
        {
            // 上位クラス／パラメーターの参照を保持
            Instance.DFUProcess = process;
            Instance.Parameter = parameter;

            // 転送処理を起動
            Instance.InvokeDFUProcess();
        }

        //
        // 内部処理
        //
        private void InvokeDFUProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.UploadProcess;

            // DFU処理開始時の画面処理
            int maximum = 100 + DFUProcessConst.DFU_WAITING_SEC_ESTIMATED;
            DFUProcess.NotifyDFUProcessStarting(maximum);

            // DFU本処理を開始（処理の終了は、処理進捗画面に通知される）
            // SMPサービスに接続
            SMPService.ConnectBLESMPService(OnConnectedToSMPService);
        }

        private void OnConnectedToSMPService(bool success)
        {
            if (success == false) {
                // 接続失敗時は転送処理を開始しない
                OnTerminatedDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_CONNECT_FAILED);
                return;
            }

            // イベントを登録
            SMPService.OnDataReceived += OnDataReceived;
            SMPService.OnTransactionFailed += OnTransactionFailed;

            // 転送処理を開始する
            DoRequestGetSlotInfo();
        }

        private void OnTerminatedDFUTransferProcess(bool success, string message)
        {
            if (success == false) {
                // エラーメッセージ文言を画面とログに出力
                Parameter.ErrorMessage = message;
                AppLogUtil.OutputLogError(message);
            }
            DFUProcess.OnTerminatedTransferProcess(success);

            // イベントを解除
            SMPService.OnDataReceived -= OnDataReceived;
            SMPService.OnTransactionFailed -= OnTransactionFailed;
        }

        //
        // スロット照会
        //
        private void DoRequestGetSlotInfo()
        {
            // DFU実行開始を通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            // リクエストデータを生成
            byte[] bodyBytes = new byte[] { 0xbf, 0xff };
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = BuildSMPHeader(OP_READ_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_STATE);

            // リクエストデータを送信
            Parameter.Command = BLEDFUCommand.GetSlotInfo;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseGetSlotInfo(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            if (CheckSlotInfo(responseData) == false) {
                OnTerminatedDFUTransferProcess(false, AppCommon.MSG_DFU_SLOT_INFO_GET_FAILED);
                return;
            }

            // 転送済みバイト数を事前にクリア
            Parameter.ImageBytesSent = 0;

            // 転送処理に移行
            DoTransferProcess();
        }

        private bool CheckSlotInfo(byte[] responseData)
        {
            // TODO: 仮の実装です。
            return true;
        }

        private byte[] BuildSMPHeader(byte op, byte flags, ushort len, ushort group, byte seq, byte id_int)
        {
            byte[] header = {
                op,
                flags,
                (byte)(len >> 8),   (byte)(len & 0xff),
                (byte)(group >> 8), (byte)(group & 0xff),
                seq,
                id_int
            };
            return header;
        }

        //
        // データ送受信時の処理
        //
        private void SendSMPRequestData(byte[] requestBody, byte[] requestHeader)
        {
            // ヘッダーと本体を連結
            byte[] requestData = Enumerable.Concat(requestHeader, requestBody).ToArray();

            // リクエストデータを送信
            SMPService.Send(requestData);

            // ログ出力
            if (Parameter.Command != BLEDFUCommand.UploadImage) {
                string dump = AppLogUtil.DumpMessage(requestData, requestData.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "Transmit SMP request ({0} bytes)\r\n{1}",
                    requestData.Length, dump));
            }

            // 応答タイムアウト監視開始
            responseTimer.Start();
        }

        // 受信済みデータ／バイト数を保持
        private byte[] ResponseData = new byte[0];
        private int received = 0;
        private int totalSize = 0;

        private void OnDataReceived(byte[] receivedData)
        {
            // 応答タイムアウト監視終了
            responseTimer.Stop();

            // ログ出力
            if (Parameter.Command != BLEDFUCommand.UploadImage) {
                string dump = AppLogUtil.DumpMessage(receivedData, receivedData.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "Incoming SMP response ({0} bytes)\r\n{1}",
                    receivedData.Length, dump));
            }

            // 受信したレスポンスデータを保持
            int responseSize = receivedData.Length;
            if (received == 0) {
                // レスポンスヘッダーからデータ長を抽出
                totalSize = GetSmpResponseBodySize(receivedData);
                // 受信済みデータを保持
                received = responseSize - SMP_HEADER_SIZE;
                ResponseData = new byte[received];
                Array.Copy(receivedData, SMP_HEADER_SIZE, ResponseData, 0, received);

            } else {
                // 受信済みデータに連結
                received += responseSize;
                ResponseData = ResponseData.Concat(receivedData).ToArray();
            }

            // 全フレームを受信したら、レスポンス処理を実行
            if (received == totalSize) {
                // 処理区分に応じて分岐
                switch (Parameter.Command) {
                case BLEDFUCommand.GetSlotInfo:
                    DoResponseGetSlotInfo(ResponseData);
                    break;
                default:
                    break;
                }
                received = 0;
                totalSize = 0;
            }
        }

        private int GetSmpResponseBodySize(byte[] responseData)
        {
            // レスポンスヘッダーの３・４バイト目からデータ長を抽出
            int totalSize = ((responseData[2] << 8) & 0xff00) + (responseData[3] & 0x00ff);
            return totalSize;
        }

        //
        // 応答失敗時の処理
        //
        private void OnTransactionFailed()
        {
            // 応答タイムアウト監視終了
            responseTimer.Stop();

            // 処理区分に応じて分岐
            string errorMessage = AppCommon.MSG_NONE;
            switch (Parameter.Command) {
            case BLEDFUCommand.GetSlotInfo:
                errorMessage = AppCommon.MSG_DFU_SLOT_INFO_GET_FAILED;
                break;
            default:
                break;
            }

            // 画面に異常終了を通知
            OnTerminatedDFUTransferProcess(false, errorMessage);
        }

        //
        // 応答タイムアウト時の処理
        //
        private void OnResponseTimerElapsed(object sender, EventArgs e)
        {
            // 応答タイムアウトを通知
            OnTerminatedDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_TIMEOUT);
        }

        //
        // SMPトランザクションに関する定義
        //
        private const int OP_READ_REQ = 0;

        private const int GRP_IMG_MGMT = 1;
        private const int CMD_IMG_MGMT_STATE = 0;

        private const int SMP_HEADER_SIZE = 8;

        //
        // TODO: 仮の実装です。
        //
        private void DoTransferProcess()
        {
            System.Threading.Thread.Sleep(2000);

            DFUProcess.NotifyDFUTransferring(true);
            for (int percentage = 0; Parameter.Status == DFUStatus.UploadProcess && percentage < 100; percentage++) {
                string progressMessage = string.Format(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
                DFUProcess.NotifyDFUProgress(progressMessage, percentage);
                System.Threading.Thread.Sleep(100);
            }
            DFUProcess.NotifyDFUTransferring(false);

            if (Parameter.Status == DFUStatus.Canceled) {
                OnTerminatedDFUTransferProcess(false, AppCommon.MSG_NONE);
            } else {
                OnTerminatedDFUTransferProcess(true, AppCommon.MSG_NONE);
            }
        }
    }
}
