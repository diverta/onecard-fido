using MaintenanceToolCommon;
using System;
using System.Timers;

namespace MaintenanceToolGUI
{
    public class CommandTimer
    {
        // タイマーオブジェクト
        private Timer timer = null;

        // タイムアウトイベント
        public delegate void CommandTimeoutEventHandler(object sender, EventArgs e);
        public event CommandTimeoutEventHandler CommandTimeoutEvent;

        // タイマー名称
        //（個別のタイマースレッドを識別するための名称）
        private readonly string timerName;

        public CommandTimer(string n, int ms)
        {
            timerName = n;
            timer = new Timer(ms);
        }

        public void Start()
        {
            timer.Elapsed += CommandTimerElapsed;
            timer.Start();
        }

        public void Stop()
        {
            timer.Stop();
            timer.Elapsed -= CommandTimerElapsed;
        }

        private void CommandTimerElapsed(object sender, EventArgs e)
        {
            try {
                // イベントを送出
                CommandTimeoutEvent(sender, e);
                AppCommon.OutputLogToFile(string.Format("CommandTimer({0}) timed out", timerName), true);

            } catch (Exception ex) {
                AppCommon.OutputLogToFile(string.Format("CommandTimer({0}): {1}", timerName, ex.Message), true);

            } finally {
                Stop();
            }
        }
    }
}
