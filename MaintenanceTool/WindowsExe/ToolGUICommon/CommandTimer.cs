using System;
using System.Timers;

namespace ToolGUICommon
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
                AppUtil.OutputLogError(string.Format("CommandTimer({0}) timed out", timerName));

            } catch (Exception ex) {
                AppUtil.OutputLogError(string.Format("CommandTimer({0}): {1}", timerName, ex.Message));

            } finally {
                Stop();
            }
        }
    }
}
