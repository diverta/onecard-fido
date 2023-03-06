using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// AccountSelectWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class AccountSelectWindow : Window
    {
        public AccountSelectWindow()
        {
            InitializeComponent();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
