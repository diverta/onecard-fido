using System.ComponentModel;

namespace MaintenanceToolApp
{
    internal class UtilityWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;

        protected void RaisePropertyChanged(string propertyName)
        {
            // 変更をViewに通知
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        // コマンドクラスの参照を保持
        public CommonWindowCancelCommand UtilityWindowCancelCommandRef { get; set; }

        public UtilityWindowViewModel()
        {
            // コマンドクラスを生成
            UtilityWindowCancelCommandRef = new CommonWindowCancelCommand();
        }
    }
}
