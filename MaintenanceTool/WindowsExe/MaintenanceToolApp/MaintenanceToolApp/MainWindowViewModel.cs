using System.ComponentModel;

namespace MaintenanceToolApp
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;

        protected void RaisePropertyChanged(string propertyName)
        {
            // 変更をViewに通知
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public MainWindowViewModel()
        {
        }

        // メイン画面のタイトル
        public static string Title {
            get { return AppCommon.MSG_TOOL_TITLE; }
            set { }
        }

        // メッセージ表示用テキストボックス
        private string messageText = string.Empty;
        public string MessageText {
            get { 
                return messageText;
            }
            set {
                if (value != null) {
                    messageText = value;
                    RaisePropertyChanged(nameof(MessageText));
                }
            }
        }
    }
}
