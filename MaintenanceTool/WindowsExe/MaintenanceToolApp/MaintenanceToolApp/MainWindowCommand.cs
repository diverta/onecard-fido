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

            // TODO: 仮の実装です。
            Window w = (Window)parameter;
            DialogUtil.ShowWarningMessage(w, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
        }
    }

    public class MainWindowQuitCommand : ICommand
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

            // メイン画面を閉じる
            Window w = (Window)parameter;
            w.Close();
        }
    }
}
