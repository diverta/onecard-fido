using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    public class PIVParameter
    {
        public Command Command { get; set; }
        public string CommandTitle { get; set; }
        public string CommandDesc { get; set; }
        public string PkeyFilePath1 { get; set; }
        public string CertFilePath1 { get; set; }
        public string PkeyFilePath2 { get; set; }
        public string CertFilePath2 { get; set; }
        public string PkeyFilePath3 { get; set; }
        public string CertFilePath3 { get; set; }
        public string AuthPin { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }

        public PIVParameter()
        {
            Command = Command.COMMAND_NONE;
            CommandTitle = string.Empty;
            CommandDesc = string.Empty;
            PkeyFilePath1 = string.Empty;
            CertFilePath1 = string.Empty;
            PkeyFilePath2 = string.Empty;
            CertFilePath2 = string.Empty;
            PkeyFilePath3 = string.Empty;
            CertFilePath3 = string.Empty;
            AuthPin = string.Empty;
            CurrentPin = string.Empty;
            NewPin = string.Empty;
        }

        public override string ToString()
        {
            string command = string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
            string pkeyCertParam = string.Format("PkeyFilePath1:{0} CertFilePath1:{1} PkeyFilePath2:{2} CertFilePath2:{3} PkeyFilePath3:{4} CertFilePath3:{5} AuthPin:{6}",
                PkeyFilePath1, CertFilePath1, PkeyFilePath2, CertFilePath2, PkeyFilePath3, CertFilePath3, AuthPin);
            string pinCommandParam = string.Format("CurrentPin:{0} NewPin:{1}", CurrentPin, NewPin);
            return string.Format("{0}\n{1}\n{2}", command, pkeyCertParam, pinCommandParam);
        }
    }

    public class PIVProcess
    {
    }
}
