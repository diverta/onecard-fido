using System;
using System.Windows;
using System.Windows.Input;

namespace MaintenanceToolApp
{
    public class MainWindowCommand : ICommand
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
