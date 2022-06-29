namespace MaintenanceToolGUI
{
    class ToolPIVCcid
    {
        // CCID I/Fからデータ受信時のイベント
        public delegate void CcidCommandTerminatedEvent(bool success);
        public event CcidCommandTerminatedEvent OnCcidCommandTerminated;

        public delegate void CcidCommandNotifyErrorMessageEvent(string errorMessage);
        public event CcidCommandNotifyErrorMessageEvent OnCcidCommandNotifyErrorMessage;
    }
}
