using System;
using System.Windows;
using System.Windows.Input;

namespace MaintenanceToolApp
{
    public class MainWindowUtilityCommand : ICommand
    {
        public event EventHandler? CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public bool CanExecute(object? parameter)
        {
            return true;
        }

        public void Execute(object? parameter)
        {
            if (parameter == null) {
                throw new ArgumentNullException(nameof(parameter));
            }

            // ユーティリティー画面を開き、実行コマンド種別をモデルに設定
            UtilityWindow w = new UtilityWindow();
            w.ShowDialogWithOwner((Window)parameter);
        }
    }
}
