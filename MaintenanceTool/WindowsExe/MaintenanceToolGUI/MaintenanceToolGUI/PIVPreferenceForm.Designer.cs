namespace MaintenanceToolGUI
{
    partial class PIVPreferenceForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tabPreference = new System.Windows.Forms.TabControl();
            this.tabPagePkeyCertManagement = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.textPinConfirm = new System.Windows.Forms.TextBox();
            this.textPin = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.buttonCertFolderPath = new System.Windows.Forms.Button();
            this.buttonPkeyFolderPath = new System.Windows.Forms.Button();
            this.textCertFolderPath = new System.Windows.Forms.TextBox();
            this.textPkeyFolderPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBoxPkeySlotId = new System.Windows.Forms.GroupBox();
            this.radioPkeySlotId3 = new System.Windows.Forms.RadioButton();
            this.radioPkeySlotId2 = new System.Windows.Forms.RadioButton();
            this.radioPkeySlotId1 = new System.Windows.Forms.RadioButton();
            this.buttonInstallPkeyCert = new System.Windows.Forms.Button();
            this.tabPagePinManagement = new System.Windows.Forms.TabPage();
            this.buttonPerformPinCommand = new System.Windows.Forms.Button();
            this.groupBoxPinText = new System.Windows.Forms.GroupBox();
            this.textNewPinConf = new System.Windows.Forms.TextBox();
            this.labelNewPinConf = new System.Windows.Forms.Label();
            this.textNewPin = new System.Windows.Forms.TextBox();
            this.textCurPin = new System.Windows.Forms.TextBox();
            this.labelNewPin = new System.Windows.Forms.Label();
            this.labelCurPin = new System.Windows.Forms.Label();
            this.groupBoxPinCommand = new System.Windows.Forms.GroupBox();
            this.radioPinCommand3 = new System.Windows.Forms.RadioButton();
            this.radioPinCommand2 = new System.Windows.Forms.RadioButton();
            this.radioPinCommand1 = new System.Windows.Forms.RadioButton();
            this.buttonPivStatus = new System.Windows.Forms.Button();
            this.buttonClearSetting = new System.Windows.Forms.Button();
            this.buttonFirmwareReset = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.buttonInitialSetting = new System.Windows.Forms.Button();
            this.tabPreference.SuspendLayout();
            this.tabPagePkeyCertManagement.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBoxPkeySlotId.SuspendLayout();
            this.tabPagePinManagement.SuspendLayout();
            this.groupBoxPinText.SuspendLayout();
            this.groupBoxPinCommand.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabPreference
            // 
            this.tabPreference.Controls.Add(this.tabPagePkeyCertManagement);
            this.tabPreference.Controls.Add(this.tabPagePinManagement);
            this.tabPreference.Location = new System.Drawing.Point(12, 11);
            this.tabPreference.Name = "tabPreference";
            this.tabPreference.SelectedIndex = 0;
            this.tabPreference.Size = new System.Drawing.Size(460, 362);
            this.tabPreference.TabIndex = 0;
            // 
            // tabPagePkeyCertManagement
            // 
            this.tabPagePkeyCertManagement.Controls.Add(this.groupBox4);
            this.tabPagePkeyCertManagement.Controls.Add(this.groupBox3);
            this.tabPagePkeyCertManagement.Controls.Add(this.groupBoxPkeySlotId);
            this.tabPagePkeyCertManagement.Controls.Add(this.buttonInstallPkeyCert);
            this.tabPagePkeyCertManagement.Location = new System.Drawing.Point(4, 22);
            this.tabPagePkeyCertManagement.Name = "tabPagePkeyCertManagement";
            this.tabPagePkeyCertManagement.Padding = new System.Windows.Forms.Padding(3);
            this.tabPagePkeyCertManagement.Size = new System.Drawing.Size(452, 336);
            this.tabPagePkeyCertManagement.TabIndex = 0;
            this.tabPagePkeyCertManagement.Text = "鍵・証明書管理";
            this.tabPagePkeyCertManagement.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.textPinConfirm);
            this.groupBox4.Controls.Add(this.textPin);
            this.groupBox4.Controls.Add(this.label5);
            this.groupBox4.Controls.Add(this.label10);
            this.groupBox4.Location = new System.Drawing.Point(18, 191);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(417, 88);
            this.groupBox4.TabIndex = 22;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "認証情報";
            // 
            // textPinConfirm
            // 
            this.textPinConfirm.Location = new System.Drawing.Point(130, 54);
            this.textPinConfirm.Name = "textPinConfirm";
            this.textPinConfirm.Size = new System.Drawing.Size(130, 19);
            this.textPinConfirm.TabIndex = 26;
            this.textPinConfirm.UseSystemPasswordChar = true;
            // 
            // textPin
            // 
            this.textPin.Location = new System.Drawing.Point(130, 27);
            this.textPin.Name = "textPin";
            this.textPin.Size = new System.Drawing.Size(130, 19);
            this.textPin.TabIndex = 24;
            this.textPin.UseSystemPasswordChar = true;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(13, 57);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(100, 12);
            this.label5.TabIndex = 25;
            this.label5.Text = "PIN番号（確認）";
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(13, 30);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(100, 12);
            this.label10.TabIndex = 23;
            this.label10.Text = "PIN番号";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.buttonCertFolderPath);
            this.groupBox3.Controls.Add(this.buttonPkeyFolderPath);
            this.groupBox3.Controls.Add(this.textCertFolderPath);
            this.groupBox3.Controls.Add(this.textPkeyFolderPath);
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Location = new System.Drawing.Point(18, 88);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(417, 88);
            this.groupBox3.TabIndex = 15;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "鍵・証明書ファイルのパス";
            // 
            // buttonCertFolderPath
            // 
            this.buttonCertFolderPath.Location = new System.Drawing.Point(352, 50);
            this.buttonCertFolderPath.Name = "buttonCertFolderPath";
            this.buttonCertFolderPath.Size = new System.Drawing.Size(55, 23);
            this.buttonCertFolderPath.TabIndex = 21;
            this.buttonCertFolderPath.Text = "参照";
            this.buttonCertFolderPath.UseVisualStyleBackColor = true;
            // 
            // buttonPkeyFolderPath
            // 
            this.buttonPkeyFolderPath.Location = new System.Drawing.Point(352, 23);
            this.buttonPkeyFolderPath.Name = "buttonPkeyFolderPath";
            this.buttonPkeyFolderPath.Size = new System.Drawing.Size(55, 23);
            this.buttonPkeyFolderPath.TabIndex = 18;
            this.buttonPkeyFolderPath.Text = "参照";
            this.buttonPkeyFolderPath.UseVisualStyleBackColor = true;
            // 
            // textCertFolderPath
            // 
            this.textCertFolderPath.Location = new System.Drawing.Point(110, 52);
            this.textCertFolderPath.Name = "textCertFolderPath";
            this.textCertFolderPath.ReadOnly = true;
            this.textCertFolderPath.Size = new System.Drawing.Size(235, 19);
            this.textCertFolderPath.TabIndex = 20;
            this.textCertFolderPath.TabStop = false;
            // 
            // textPkeyFolderPath
            // 
            this.textPkeyFolderPath.Location = new System.Drawing.Point(110, 25);
            this.textPkeyFolderPath.Name = "textPkeyFolderPath";
            this.textPkeyFolderPath.ReadOnly = true;
            this.textPkeyFolderPath.Size = new System.Drawing.Size(235, 19);
            this.textPkeyFolderPath.TabIndex = 17;
            this.textPkeyFolderPath.TabStop = false;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(13, 55);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(85, 12);
            this.label2.TabIndex = 19;
            this.label2.Text = "証明書ファイル";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(13, 28);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(85, 12);
            this.label3.TabIndex = 16;
            this.label3.Text = "鍵ファイル";
            // 
            // groupBoxPkeySlotId
            // 
            this.groupBoxPkeySlotId.Controls.Add(this.radioPkeySlotId3);
            this.groupBoxPkeySlotId.Controls.Add(this.radioPkeySlotId2);
            this.groupBoxPkeySlotId.Controls.Add(this.radioPkeySlotId1);
            this.groupBoxPkeySlotId.Location = new System.Drawing.Point(18, 20);
            this.groupBoxPkeySlotId.Name = "groupBoxPkeySlotId";
            this.groupBoxPkeySlotId.Size = new System.Drawing.Size(417, 53);
            this.groupBoxPkeySlotId.TabIndex = 1;
            this.groupBoxPkeySlotId.TabStop = false;
            this.groupBoxPkeySlotId.Text = "インストールする鍵・証明書";
            // 
            // radioPkeySlotId3
            // 
            this.radioPkeySlotId3.AutoSize = true;
            this.radioPkeySlotId3.Location = new System.Drawing.Point(205, 23);
            this.radioPkeySlotId3.Name = "radioPkeySlotId3";
            this.radioPkeySlotId3.Size = new System.Drawing.Size(83, 16);
            this.radioPkeySlotId3.TabIndex = 10;
            this.radioPkeySlotId3.Text = "管理機能用";
            this.radioPkeySlotId3.UseVisualStyleBackColor = true;
            // 
            // radioPkeySlotId2
            // 
            this.radioPkeySlotId2.AutoSize = true;
            this.radioPkeySlotId2.Location = new System.Drawing.Point(110, 23);
            this.radioPkeySlotId2.Name = "radioPkeySlotId2";
            this.radioPkeySlotId2.Size = new System.Drawing.Size(83, 16);
            this.radioPkeySlotId2.TabIndex = 9;
            this.radioPkeySlotId2.Text = "電子署名用";
            this.radioPkeySlotId2.UseVisualStyleBackColor = true;
            // 
            // radioPkeySlotId1
            // 
            this.radioPkeySlotId1.AutoSize = true;
            this.radioPkeySlotId1.Checked = true;
            this.radioPkeySlotId1.Location = new System.Drawing.Point(18, 23);
            this.radioPkeySlotId1.Name = "radioPkeySlotId1";
            this.radioPkeySlotId1.Size = new System.Drawing.Size(77, 16);
            this.radioPkeySlotId1.TabIndex = 8;
            this.radioPkeySlotId1.TabStop = true;
            this.radioPkeySlotId1.Text = "PIV認証用";
            this.radioPkeySlotId1.UseVisualStyleBackColor = true;
            // 
            // buttonInstallPkeyCert
            // 
            this.buttonInstallPkeyCert.Location = new System.Drawing.Point(126, 295);
            this.buttonInstallPkeyCert.Name = "buttonInstallPkeyCert";
            this.buttonInstallPkeyCert.Size = new System.Drawing.Size(200, 23);
            this.buttonInstallPkeyCert.TabIndex = 27;
            this.buttonInstallPkeyCert.Text = "鍵・証明書ファイルのインストール";
            this.buttonInstallPkeyCert.UseVisualStyleBackColor = true;
            // 
            // tabPagePinManagement
            // 
            this.tabPagePinManagement.Controls.Add(this.buttonPerformPinCommand);
            this.tabPagePinManagement.Controls.Add(this.groupBoxPinText);
            this.tabPagePinManagement.Controls.Add(this.groupBoxPinCommand);
            this.tabPagePinManagement.Location = new System.Drawing.Point(4, 22);
            this.tabPagePinManagement.Name = "tabPagePinManagement";
            this.tabPagePinManagement.Padding = new System.Windows.Forms.Padding(3);
            this.tabPagePinManagement.Size = new System.Drawing.Size(452, 336);
            this.tabPagePinManagement.TabIndex = 1;
            this.tabPagePinManagement.Text = "PIN番号管理";
            this.tabPagePinManagement.UseVisualStyleBackColor = true;
            // 
            // buttonPerformPinCommand
            // 
            this.buttonPerformPinCommand.Location = new System.Drawing.Point(146, 215);
            this.buttonPerformPinCommand.Name = "buttonPerformPinCommand";
            this.buttonPerformPinCommand.Size = new System.Drawing.Size(160, 23);
            this.buttonPerformPinCommand.TabIndex = 19;
            this.buttonPerformPinCommand.Text = "実行";
            this.buttonPerformPinCommand.UseVisualStyleBackColor = true;
            // 
            // groupBoxPinText
            // 
            this.groupBoxPinText.Controls.Add(this.textNewPinConf);
            this.groupBoxPinText.Controls.Add(this.labelNewPinConf);
            this.groupBoxPinText.Controls.Add(this.textNewPin);
            this.groupBoxPinText.Controls.Add(this.textCurPin);
            this.groupBoxPinText.Controls.Add(this.labelNewPin);
            this.groupBoxPinText.Controls.Add(this.labelCurPin);
            this.groupBoxPinText.Location = new System.Drawing.Point(18, 88);
            this.groupBoxPinText.Name = "groupBoxPinText";
            this.groupBoxPinText.Size = new System.Drawing.Size(417, 114);
            this.groupBoxPinText.TabIndex = 18;
            this.groupBoxPinText.TabStop = false;
            this.groupBoxPinText.Text = "認証情報";
            // 
            // textNewPinConf
            // 
            this.textNewPinConf.Location = new System.Drawing.Point(165, 77);
            this.textNewPinConf.MaxLength = 64;
            this.textNewPinConf.Name = "textNewPinConf";
            this.textNewPinConf.Size = new System.Drawing.Size(130, 19);
            this.textNewPinConf.TabIndex = 22;
            this.textNewPinConf.UseSystemPasswordChar = true;
            // 
            // labelNewPinConf
            // 
            this.labelNewPinConf.Location = new System.Drawing.Point(13, 80);
            this.labelNewPinConf.Name = "labelNewPinConf";
            this.labelNewPinConf.Size = new System.Drawing.Size(140, 12);
            this.labelNewPinConf.TabIndex = 21;
            this.labelNewPinConf.Text = "新しいPUK番号（確認）";
            // 
            // textNewPin
            // 
            this.textNewPin.Location = new System.Drawing.Point(165, 50);
            this.textNewPin.MaxLength = 64;
            this.textNewPin.Name = "textNewPin";
            this.textNewPin.Size = new System.Drawing.Size(130, 19);
            this.textNewPin.TabIndex = 12;
            this.textNewPin.UseSystemPasswordChar = true;
            // 
            // textCurPin
            // 
            this.textCurPin.Location = new System.Drawing.Point(165, 23);
            this.textCurPin.MaxLength = 64;
            this.textCurPin.Name = "textCurPin";
            this.textCurPin.Size = new System.Drawing.Size(130, 19);
            this.textCurPin.TabIndex = 11;
            this.textCurPin.UseSystemPasswordChar = true;
            // 
            // labelNewPin
            // 
            this.labelNewPin.Location = new System.Drawing.Point(13, 53);
            this.labelNewPin.Name = "labelNewPin";
            this.labelNewPin.Size = new System.Drawing.Size(140, 12);
            this.labelNewPin.TabIndex = 20;
            this.labelNewPin.Text = "新しいPUK番号";
            // 
            // labelCurPin
            // 
            this.labelCurPin.Location = new System.Drawing.Point(13, 26);
            this.labelCurPin.Name = "labelCurPin";
            this.labelCurPin.Size = new System.Drawing.Size(140, 12);
            this.labelCurPin.TabIndex = 19;
            this.labelCurPin.Text = "現在のPUK番号";
            // 
            // groupBoxPinCommand
            // 
            this.groupBoxPinCommand.Controls.Add(this.radioPinCommand3);
            this.groupBoxPinCommand.Controls.Add(this.radioPinCommand2);
            this.groupBoxPinCommand.Controls.Add(this.radioPinCommand1);
            this.groupBoxPinCommand.Location = new System.Drawing.Point(18, 20);
            this.groupBoxPinCommand.Name = "groupBoxPinCommand";
            this.groupBoxPinCommand.Size = new System.Drawing.Size(417, 53);
            this.groupBoxPinCommand.TabIndex = 15;
            this.groupBoxPinCommand.TabStop = false;
            this.groupBoxPinCommand.Text = "実行する機能";
            // 
            // radioPinCommand3
            // 
            this.radioPinCommand3.AutoSize = true;
            this.radioPinCommand3.Location = new System.Drawing.Point(236, 23);
            this.radioPinCommand3.Name = "radioPinCommand3";
            this.radioPinCommand3.Size = new System.Drawing.Size(106, 16);
            this.radioPinCommand3.TabIndex = 16;
            this.radioPinCommand3.Text = "PIN番号をリセット";
            this.radioPinCommand3.UseVisualStyleBackColor = true;
            // 
            // radioPinCommand2
            // 
            this.radioPinCommand2.AutoSize = true;
            this.radioPinCommand2.Location = new System.Drawing.Point(128, 23);
            this.radioPinCommand2.Name = "radioPinCommand2";
            this.radioPinCommand2.Size = new System.Drawing.Size(102, 16);
            this.radioPinCommand2.TabIndex = 15;
            this.radioPinCommand2.Text = "PUK番号を変更";
            this.radioPinCommand2.UseVisualStyleBackColor = true;
            // 
            // radioPinCommand1
            // 
            this.radioPinCommand1.AutoSize = true;
            this.radioPinCommand1.Checked = true;
            this.radioPinCommand1.Location = new System.Drawing.Point(18, 23);
            this.radioPinCommand1.Name = "radioPinCommand1";
            this.radioPinCommand1.Size = new System.Drawing.Size(98, 16);
            this.radioPinCommand1.TabIndex = 14;
            this.radioPinCommand1.TabStop = true;
            this.radioPinCommand1.Text = "PIN番号を変更";
            this.radioPinCommand1.UseVisualStyleBackColor = true;
            // 
            // buttonPivStatus
            // 
            this.buttonPivStatus.Location = new System.Drawing.Point(16, 385);
            this.buttonPivStatus.Name = "buttonPivStatus";
            this.buttonPivStatus.Size = new System.Drawing.Size(110, 23);
            this.buttonPivStatus.TabIndex = 1;
            this.buttonPivStatus.Text = "設定情報を参照";
            this.buttonPivStatus.UseVisualStyleBackColor = true;
            // 
            // buttonClearSetting
            // 
            this.buttonClearSetting.Location = new System.Drawing.Point(238, 385);
            this.buttonClearSetting.Name = "buttonClearSetting";
            this.buttonClearSetting.Size = new System.Drawing.Size(110, 23);
            this.buttonClearSetting.TabIndex = 2;
            this.buttonClearSetting.Text = "設定情報を消去";
            this.buttonClearSetting.UseVisualStyleBackColor = true;
            // 
            // buttonFirmwareReset
            // 
            this.buttonFirmwareReset.Location = new System.Drawing.Point(358, 385);
            this.buttonFirmwareReset.Name = "buttonFirmwareReset";
            this.buttonFirmwareReset.Size = new System.Drawing.Size(110, 23);
            this.buttonFirmwareReset.TabIndex = 3;
            this.buttonFirmwareReset.Text = "認証器のリセット";
            this.buttonFirmwareReset.UseVisualStyleBackColor = true;
            // 
            // buttonClose
            // 
            this.buttonClose.Location = new System.Drawing.Point(202, 419);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(80, 23);
            this.buttonClose.TabIndex = 4;
            this.buttonClose.Text = "閉じる";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // buttonInitialSetting
            // 
            this.buttonInitialSetting.Location = new System.Drawing.Point(132, 385);
            this.buttonInitialSetting.Name = "buttonInitialSetting";
            this.buttonInitialSetting.Size = new System.Drawing.Size(100, 23);
            this.buttonInitialSetting.TabIndex = 5;
            this.buttonInitialSetting.Text = "ID設定を実行";
            this.buttonInitialSetting.UseVisualStyleBackColor = true;
            // 
            // PIVPreferenceForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 454);
            this.Controls.Add(this.buttonInitialSetting);
            this.Controls.Add(this.buttonFirmwareReset);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonClearSetting);
            this.Controls.Add(this.buttonPivStatus);
            this.Controls.Add(this.tabPreference);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PIVPreferenceForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "PIV機能設定";
            this.tabPreference.ResumeLayout(false);
            this.tabPagePkeyCertManagement.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBoxPkeySlotId.ResumeLayout(false);
            this.groupBoxPkeySlotId.PerformLayout();
            this.tabPagePinManagement.ResumeLayout(false);
            this.groupBoxPinText.ResumeLayout(false);
            this.groupBoxPinText.PerformLayout();
            this.groupBoxPinCommand.ResumeLayout(false);
            this.groupBoxPinCommand.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabPreference;
        private System.Windows.Forms.TabPage tabPagePkeyCertManagement;
        private System.Windows.Forms.TabPage tabPagePinManagement;
        private System.Windows.Forms.Button buttonInstallPkeyCert;
        private System.Windows.Forms.GroupBox groupBoxPkeySlotId;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.TextBox textPinConfirm;
        private System.Windows.Forms.TextBox textPin;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button buttonCertFolderPath;
        private System.Windows.Forms.Button buttonPkeyFolderPath;
        private System.Windows.Forms.TextBox textCertFolderPath;
        private System.Windows.Forms.TextBox textPkeyFolderPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button buttonPivStatus;
        private System.Windows.Forms.Button buttonClearSetting;
        private System.Windows.Forms.Button buttonFirmwareReset;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.Button buttonPerformPinCommand;
        private System.Windows.Forms.GroupBox groupBoxPinText;
        private System.Windows.Forms.TextBox textNewPinConf;
        private System.Windows.Forms.Label labelNewPinConf;
        private System.Windows.Forms.TextBox textNewPin;
        private System.Windows.Forms.TextBox textCurPin;
        private System.Windows.Forms.Label labelNewPin;
        private System.Windows.Forms.Label labelCurPin;
        private System.Windows.Forms.GroupBox groupBoxPinCommand;
        private System.Windows.Forms.RadioButton radioPinCommand3;
        private System.Windows.Forms.RadioButton radioPinCommand2;
        private System.Windows.Forms.RadioButton radioPinCommand1;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.RadioButton radioPkeySlotId3;
        private System.Windows.Forms.RadioButton radioPkeySlotId2;
        private System.Windows.Forms.RadioButton radioPkeySlotId1;
        private System.Windows.Forms.Button buttonInitialSetting;
    }
}