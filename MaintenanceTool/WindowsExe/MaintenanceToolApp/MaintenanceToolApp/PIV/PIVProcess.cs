using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    public class PIVParameter
    {
        public Command Command { get; set; }
        public string CommandTitle { get; set; }
        public string CommandDesc { get; set; }

        public PIVParameter()
        {
            Command = Command.COMMAND_NONE;
            CommandTitle = string.Empty;
            CommandDesc = string.Empty;
        }

        public override string ToString()
        {
            string command = string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
            return string.Format("{0}", command);
        }
    }

    public class PIVProcess
    {
    }
}
