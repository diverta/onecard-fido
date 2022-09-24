using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.FIDOSettings
{
    public class FIDOSettingsParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public string PinNew { get; set; }
        public string PinOld { get; set; }

        public FIDOSettingsParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            PinNew = string.Empty;
            PinOld = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("Command:{0} CommandTitle:{1} PinNew:{2} PinOld:{3}", Command, CommandTitle, PinNew, PinOld);
        }
    }

    public class FIDOSettingsProcess
    {
    }
}
