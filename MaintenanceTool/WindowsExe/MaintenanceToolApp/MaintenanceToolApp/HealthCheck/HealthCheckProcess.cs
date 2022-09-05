using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.HealthCheck
{
    public class HealthCheckParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public Transport Transport { get; set; }
        public string Pin { get; set; }

        public HealthCheckParameter() 
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            Transport = Transport.TRANSPORT_NONE;
            Pin = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("Command:{0} CommandTitle:{1} Transport:{2} Pin:{3}", Command, CommandTitle, Transport, Pin);
        }
    }

    public class HealthCheckProcess
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public HealthCheckProcess(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_BLE_CTAP2_HCHECK:
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestBleCtap2Hcheck();
                break;

            case Command.COMMAND_BLE_U2F_HCHECK:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestBleU2fHcheck();
                break;

            case Command.COMMAND_TEST_BLE_PING:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_TEST_BLE_PING;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestTestBlePing();
                break;

            case Command.COMMAND_HID_CTAP2_HCHECK:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestHidCtap2Hcheck();
                break;

            case Command.COMMAND_HID_U2F_HCHECK:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_HID_U2F_HEALTHCHECK;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestHidU2fHcheck();
                break;

            case Command.COMMAND_TEST_CTAPHID_PING:
                // 処理開始メッセージを表示
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_TEST_CTAPHID_PING;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestTestCtapHidPing();
                break;

            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        //
        // 内部処理
        //
        private void DoRequestBleCtap2Hcheck()
        {
            // BLE経由でCTAP2ヘルスチェックを実行
            new CTAP2HealthCheckProcess(Parameter).DoRequestBleCtap2HealthCheck(DoResponseU2fHealthCheck);
        }

        private void DoRequestBleU2fHcheck()
        {
            // BLE経由でU2Fヘルスチェックを実行
            new U2FHealthCheckProcess(Parameter).DoRequestBleU2fHealthCheck(DoResponseU2fHealthCheck);
        }

        private void DoRequestTestBlePing()
        {
            // BLE経由でPINGテストを実行
            new U2FHealthCheckProcess(Parameter).DoRequestBlePingTest(DoResponseU2fHealthCheck);
        }

        private void DoRequestHidCtap2Hcheck()
        {
            // HID I/F経由でCTAP2ヘルスチェックを実行
            new CTAP2HealthCheckProcess(Parameter).DoRequestHidCtap2HealthCheck(DoResponseU2fHealthCheck);
        }

        private void DoRequestHidU2fHcheck()
        {
            // HID I/F経由でU2Fヘルスチェックを実行
            new U2FHealthCheckProcess(Parameter).DoRequestHidU2fHealthCheck(DoResponseU2fHealthCheck);
        }

        private void DoRequestTestCtapHidPing()
        {
            // HID I/F経由でPINGテストを実行
            new U2FHealthCheckProcess(Parameter).DoRequestHidPingTest(DoResponseU2fHealthCheck);
        }

        //
        // 下位クラスからのコールバック
        //
        private void DoResponseU2fHealthCheck(string commandTitle, string errorMessage, bool success)
        {
            // メイン画面に制御を戻す
            CommandProcess.NotifyCommandTerminated(commandTitle, errorMessage, success, ParentWindow);
        }
    }
}
