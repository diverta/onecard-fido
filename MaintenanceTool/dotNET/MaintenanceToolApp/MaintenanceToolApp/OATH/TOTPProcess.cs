using static MaintenanceToolApp.AppDefine;

namespace MaintenanceTool.OATH
{
    public class TOTPParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public Transport Transport { get; set; }

        public TOTPParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            Transport = Transport.TRANSPORT_NONE;
        }
    }

    internal class TOTPProcess
    {
    }
}
