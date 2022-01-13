﻿namespace MaintenanceToolGUI
{
    partial class PGPPreferenceForm
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
            this.tabPagePGPKeyManagement = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.textPinConfirm = new System.Windows.Forms.TextBox();
            this.textPin = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.buttonBackupFolderPath = new System.Windows.Forms.Button();
            this.buttonPubkeyFolderPath = new System.Windows.Forms.Button();
            this.textBackupFolderPath = new System.Windows.Forms.TextBox();
            this.textPubkeyFolderPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.textComment = new System.Windows.Forms.TextBox();
            this.textMailAddress = new System.Windows.Forms.TextBox();
            this.textRealName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.checkKeyForAuth = new System.Windows.Forms.CheckBox();
            this.label7 = new System.Windows.Forms.Label();
            this.checkKeyForEncr = new System.Windows.Forms.CheckBox();
            this.label6 = new System.Windows.Forms.Label();
            this.checkKeyForSign = new System.Windows.Forms.CheckBox();
            this.buttonInstallPGPKey = new System.Windows.Forms.Button();
            this.tabPagePinManagement = new System.Windows.Forms.TabPage();
            this.button1 = new System.Windows.Forms.Button();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.textBox3 = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.radioButton3 = new System.Windows.Forms.RadioButton();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.buttonPGPStatus = new System.Windows.Forms.Button();
            this.buttonPGPReset = new System.Windows.Forms.Button();
            this.buttonFirmwareReset = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.tabPreference.SuspendLayout();
            this.tabPagePGPKeyManagement.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPagePinManagement.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabPreference
            // 
            this.tabPreference.Controls.Add(this.tabPagePGPKeyManagement);
            this.tabPreference.Controls.Add(this.tabPagePinManagement);
            this.tabPreference.Location = new System.Drawing.Point(12, 11);
            this.tabPreference.Name = "tabPreference";
            this.tabPreference.SelectedIndex = 0;
            this.tabPreference.Size = new System.Drawing.Size(460, 480);
            this.tabPreference.TabIndex = 0;
            // 
            // tabPagePGPKeyManagement
            // 
            this.tabPagePGPKeyManagement.Controls.Add(this.groupBox4);
            this.tabPagePGPKeyManagement.Controls.Add(this.groupBox3);
            this.tabPagePGPKeyManagement.Controls.Add(this.groupBox2);
            this.tabPagePGPKeyManagement.Controls.Add(this.groupBox1);
            this.tabPagePGPKeyManagement.Controls.Add(this.buttonInstallPGPKey);
            this.tabPagePGPKeyManagement.Location = new System.Drawing.Point(4, 22);
            this.tabPagePGPKeyManagement.Name = "tabPagePGPKeyManagement";
            this.tabPagePGPKeyManagement.Padding = new System.Windows.Forms.Padding(3);
            this.tabPagePGPKeyManagement.Size = new System.Drawing.Size(452, 454);
            this.tabPagePGPKeyManagement.TabIndex = 0;
            this.tabPagePGPKeyManagement.Text = "PGP鍵管理";
            this.tabPagePGPKeyManagement.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.textPinConfirm);
            this.groupBox4.Controls.Add(this.textPin);
            this.groupBox4.Controls.Add(this.label5);
            this.groupBox4.Controls.Add(this.label10);
            this.groupBox4.Location = new System.Drawing.Point(18, 320);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(417, 88);
            this.groupBox4.TabIndex = 22;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "認証情報";
            // 
            // textPinConfirm
            // 
            this.textPinConfirm.Location = new System.Drawing.Point(110, 54);
            this.textPinConfirm.Name = "textPinConfirm";
            this.textPinConfirm.Size = new System.Drawing.Size(130, 19);
            this.textPinConfirm.TabIndex = 26;
            this.textPinConfirm.UseSystemPasswordChar = true;
            // 
            // textPin
            // 
            this.textPin.Location = new System.Drawing.Point(110, 27);
            this.textPin.Name = "textPin";
            this.textPin.Size = new System.Drawing.Size(130, 19);
            this.textPin.TabIndex = 24;
            this.textPin.UseSystemPasswordChar = true;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(13, 57);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(90, 12);
            this.label5.TabIndex = 25;
            this.label5.Text = "PIN番号（確認）";
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(13, 30);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(70, 12);
            this.label10.TabIndex = 23;
            this.label10.Text = "PIN番号";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.buttonBackupFolderPath);
            this.groupBox3.Controls.Add(this.buttonPubkeyFolderPath);
            this.groupBox3.Controls.Add(this.textBackupFolderPath);
            this.groupBox3.Controls.Add(this.textPubkeyFolderPath);
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Location = new System.Drawing.Point(18, 217);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(417, 88);
            this.groupBox3.TabIndex = 15;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "各種ファイルの出力先フォルダー";
            // 
            // buttonBackupFolderPath
            // 
            this.buttonBackupFolderPath.Location = new System.Drawing.Point(352, 50);
            this.buttonBackupFolderPath.Name = "buttonBackupFolderPath";
            this.buttonBackupFolderPath.Size = new System.Drawing.Size(55, 23);
            this.buttonBackupFolderPath.TabIndex = 21;
            this.buttonBackupFolderPath.Text = "参照";
            this.buttonBackupFolderPath.UseVisualStyleBackColor = true;
            // 
            // buttonPubkeyFolderPath
            // 
            this.buttonPubkeyFolderPath.Location = new System.Drawing.Point(352, 23);
            this.buttonPubkeyFolderPath.Name = "buttonPubkeyFolderPath";
            this.buttonPubkeyFolderPath.Size = new System.Drawing.Size(55, 23);
            this.buttonPubkeyFolderPath.TabIndex = 18;
            this.buttonPubkeyFolderPath.Text = "参照";
            this.buttonPubkeyFolderPath.UseVisualStyleBackColor = true;
            // 
            // textBackupFolderPath
            // 
            this.textBackupFolderPath.Location = new System.Drawing.Point(110, 52);
            this.textBackupFolderPath.Name = "textBackupFolderPath";
            this.textBackupFolderPath.ReadOnly = true;
            this.textBackupFolderPath.Size = new System.Drawing.Size(235, 19);
            this.textBackupFolderPath.TabIndex = 20;
            this.textBackupFolderPath.TabStop = false;
            // 
            // textPubkeyFolderPath
            // 
            this.textPubkeyFolderPath.Location = new System.Drawing.Point(110, 25);
            this.textPubkeyFolderPath.Name = "textPubkeyFolderPath";
            this.textPubkeyFolderPath.ReadOnly = true;
            this.textPubkeyFolderPath.Size = new System.Drawing.Size(235, 19);
            this.textPubkeyFolderPath.TabIndex = 17;
            this.textPubkeyFolderPath.TabStop = false;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(13, 55);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(70, 12);
            this.label2.TabIndex = 19;
            this.label2.Text = "バックアップ";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(13, 28);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(70, 12);
            this.label3.TabIndex = 16;
            this.label3.Text = "PGP公開鍵";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.textComment);
            this.groupBox2.Controls.Add(this.textMailAddress);
            this.groupBox2.Controls.Add(this.textRealName);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Location = new System.Drawing.Point(18, 88);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(417, 114);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "PGP鍵の情報";
            // 
            // textComment
            // 
            this.textComment.Location = new System.Drawing.Point(110, 77);
            this.textComment.Name = "textComment";
            this.textComment.Size = new System.Drawing.Size(235, 19);
            this.textComment.TabIndex = 14;
            // 
            // textMailAddress
            // 
            this.textMailAddress.Location = new System.Drawing.Point(110, 50);
            this.textMailAddress.Name = "textMailAddress";
            this.textMailAddress.Size = new System.Drawing.Size(235, 19);
            this.textMailAddress.TabIndex = 12;
            // 
            // textRealName
            // 
            this.textRealName.Location = new System.Drawing.Point(110, 23);
            this.textRealName.Name = "textRealName";
            this.textRealName.Size = new System.Drawing.Size(235, 19);
            this.textRealName.TabIndex = 10;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(13, 80);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 12);
            this.label1.TabIndex = 13;
            this.label1.Text = "コメント";
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(13, 53);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(70, 12);
            this.label9.TabIndex = 11;
            this.label9.Text = "メールアドレス";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(13, 26);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(70, 12);
            this.label4.TabIndex = 9;
            this.label4.Text = "名前";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.checkKeyForAuth);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.checkKeyForEncr);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.checkKeyForSign);
            this.groupBox1.Location = new System.Drawing.Point(18, 20);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(417, 53);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "インストールするPGP鍵";
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(225, 26);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(50, 12);
            this.label8.TabIndex = 7;
            this.label8.Text = "認証用";
            // 
            // checkKeyForAuth
            // 
            this.checkKeyForAuth.AutoSize = true;
            this.checkKeyForAuth.Checked = true;
            this.checkKeyForAuth.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkKeyForAuth.Enabled = false;
            this.checkKeyForAuth.Location = new System.Drawing.Point(205, 25);
            this.checkKeyForAuth.Name = "checkKeyForAuth";
            this.checkKeyForAuth.Size = new System.Drawing.Size(15, 14);
            this.checkKeyForAuth.TabIndex = 6;
            this.checkKeyForAuth.UseVisualStyleBackColor = true;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(135, 26);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(60, 12);
            this.label7.TabIndex = 5;
            this.label7.Text = "暗号化用";
            // 
            // checkKeyForEncr
            // 
            this.checkKeyForEncr.AutoSize = true;
            this.checkKeyForEncr.Checked = true;
            this.checkKeyForEncr.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkKeyForEncr.Enabled = false;
            this.checkKeyForEncr.Location = new System.Drawing.Point(115, 25);
            this.checkKeyForEncr.Name = "checkKeyForEncr";
            this.checkKeyForEncr.Size = new System.Drawing.Size(15, 14);
            this.checkKeyForEncr.TabIndex = 4;
            this.checkKeyForEncr.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(35, 26);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(70, 12);
            this.label6.TabIndex = 3;
            this.label6.Text = "電子署名用";
            // 
            // checkKeyForSign
            // 
            this.checkKeyForSign.AutoSize = true;
            this.checkKeyForSign.Checked = true;
            this.checkKeyForSign.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkKeyForSign.Enabled = false;
            this.checkKeyForSign.Location = new System.Drawing.Point(15, 25);
            this.checkKeyForSign.Name = "checkKeyForSign";
            this.checkKeyForSign.Size = new System.Drawing.Size(15, 14);
            this.checkKeyForSign.TabIndex = 2;
            this.checkKeyForSign.UseVisualStyleBackColor = true;
            // 
            // buttonInstallPGPKey
            // 
            this.buttonInstallPGPKey.Location = new System.Drawing.Point(146, 419);
            this.buttonInstallPGPKey.Name = "buttonInstallPGPKey";
            this.buttonInstallPGPKey.Size = new System.Drawing.Size(160, 23);
            this.buttonInstallPGPKey.TabIndex = 27;
            this.buttonInstallPGPKey.Text = "PGP秘密鍵のインストール";
            this.buttonInstallPGPKey.UseVisualStyleBackColor = true;
            // 
            // tabPagePinManagement
            // 
            this.tabPagePinManagement.Controls.Add(this.button1);
            this.tabPagePinManagement.Controls.Add(this.groupBox6);
            this.tabPagePinManagement.Controls.Add(this.groupBox5);
            this.tabPagePinManagement.Location = new System.Drawing.Point(4, 22);
            this.tabPagePinManagement.Name = "tabPagePinManagement";
            this.tabPagePinManagement.Padding = new System.Windows.Forms.Padding(3);
            this.tabPagePinManagement.Size = new System.Drawing.Size(452, 454);
            this.tabPagePinManagement.TabIndex = 1;
            this.tabPagePinManagement.Text = "PIN番号管理";
            this.tabPagePinManagement.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            this.button1.Enabled = false;
            this.button1.Location = new System.Drawing.Point(146, 216);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(160, 23);
            this.button1.TabIndex = 19;
            this.button1.Text = "実行";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.textBox3);
            this.groupBox6.Controls.Add(this.label13);
            this.groupBox6.Controls.Add(this.textBox1);
            this.groupBox6.Controls.Add(this.textBox2);
            this.groupBox6.Controls.Add(this.label11);
            this.groupBox6.Controls.Add(this.label12);
            this.groupBox6.Enabled = false;
            this.groupBox6.Location = new System.Drawing.Point(18, 88);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(417, 114);
            this.groupBox6.TabIndex = 18;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "認証情報";
            // 
            // textBox3
            // 
            this.textBox3.Location = new System.Drawing.Point(175, 77);
            this.textBox3.Name = "textBox3";
            this.textBox3.Size = new System.Drawing.Size(130, 19);
            this.textBox3.TabIndex = 22;
            this.textBox3.UseSystemPasswordChar = true;
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(13, 80);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(155, 12);
            this.label13.TabIndex = 21;
            this.label13.Text = "新しい管理用PIN番号（確認）";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(175, 50);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(130, 19);
            this.textBox1.TabIndex = 12;
            this.textBox1.UseSystemPasswordChar = true;
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(175, 23);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(130, 19);
            this.textBox2.TabIndex = 11;
            this.textBox2.UseSystemPasswordChar = true;
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(13, 53);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(155, 12);
            this.label11.TabIndex = 20;
            this.label11.Text = "新しい管理用PIN番号";
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(13, 26);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(155, 12);
            this.label12.TabIndex = 19;
            this.label12.Text = "現在の管理用PIN番号";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.radioButton3);
            this.groupBox5.Controls.Add(this.radioButton2);
            this.groupBox5.Controls.Add(this.radioButton1);
            this.groupBox5.Enabled = false;
            this.groupBox5.Location = new System.Drawing.Point(18, 20);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(417, 53);
            this.groupBox5.TabIndex = 15;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "実行する機能";
            // 
            // radioButton3
            // 
            this.radioButton3.AutoSize = true;
            this.radioButton3.Location = new System.Drawing.Point(273, 23);
            this.radioButton3.Name = "radioButton3";
            this.radioButton3.Size = new System.Drawing.Size(106, 16);
            this.radioButton3.TabIndex = 16;
            this.radioButton3.Text = "PIN番号をリセット";
            this.radioButton3.UseVisualStyleBackColor = true;
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(128, 23);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(134, 16);
            this.radioButton2.TabIndex = 15;
            this.radioButton2.Text = "管理用PIN番号を変更";
            this.radioButton2.UseVisualStyleBackColor = true;
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Checked = true;
            this.radioButton1.Location = new System.Drawing.Point(18, 23);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(98, 16);
            this.radioButton1.TabIndex = 14;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "PIN番号を変更";
            this.radioButton1.UseVisualStyleBackColor = true;
            // 
            // buttonPGPStatus
            // 
            this.buttonPGPStatus.Location = new System.Drawing.Point(16, 500);
            this.buttonPGPStatus.Name = "buttonPGPStatus";
            this.buttonPGPStatus.Size = new System.Drawing.Size(110, 23);
            this.buttonPGPStatus.TabIndex = 1;
            this.buttonPGPStatus.Text = "設定情報を参照";
            this.buttonPGPStatus.UseVisualStyleBackColor = true;
            // 
            // buttonPGPReset
            // 
            this.buttonPGPReset.Location = new System.Drawing.Point(136, 500);
            this.buttonPGPReset.Name = "buttonPGPReset";
            this.buttonPGPReset.Size = new System.Drawing.Size(110, 23);
            this.buttonPGPReset.TabIndex = 2;
            this.buttonPGPReset.Text = "設定情報を消去";
            this.buttonPGPReset.UseVisualStyleBackColor = true;
            // 
            // buttonFirmwareReset
            // 
            this.buttonFirmwareReset.Location = new System.Drawing.Point(268, 500);
            this.buttonFirmwareReset.Name = "buttonFirmwareReset";
            this.buttonFirmwareReset.Size = new System.Drawing.Size(110, 23);
            this.buttonFirmwareReset.TabIndex = 3;
            this.buttonFirmwareReset.Text = "認証器のリセット";
            this.buttonFirmwareReset.UseVisualStyleBackColor = true;
            this.buttonFirmwareReset.Click += new System.EventHandler(this.buttonFirmwareReset_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.Location = new System.Drawing.Point(388, 500);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(80, 23);
            this.buttonClose.TabIndex = 4;
            this.buttonClose.Text = "閉じる";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // PGPPreferenceForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 536);
            this.Controls.Add(this.buttonFirmwareReset);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonPGPReset);
            this.Controls.Add(this.buttonPGPStatus);
            this.Controls.Add(this.tabPreference);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PGPPreferenceForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "OpenPGP機能設定";
            this.tabPreference.ResumeLayout(false);
            this.tabPagePGPKeyManagement.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.tabPagePinManagement.ResumeLayout(false);
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabPreference;
        private System.Windows.Forms.TabPage tabPagePGPKeyManagement;
        private System.Windows.Forms.TabPage tabPagePinManagement;
        private System.Windows.Forms.Button buttonInstallPGPKey;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.CheckBox checkKeyForAuth;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.CheckBox checkKeyForEncr;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.CheckBox checkKeyForSign;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.TextBox textPinConfirm;
        private System.Windows.Forms.TextBox textPin;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button buttonBackupFolderPath;
        private System.Windows.Forms.Button buttonPubkeyFolderPath;
        private System.Windows.Forms.TextBox textBackupFolderPath;
        private System.Windows.Forms.TextBox textPubkeyFolderPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textComment;
        private System.Windows.Forms.TextBox textMailAddress;
        private System.Windows.Forms.TextBox textRealName;
        private System.Windows.Forms.Button buttonPGPStatus;
        private System.Windows.Forms.Button buttonPGPReset;
        private System.Windows.Forms.Button buttonFirmwareReset;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.RadioButton radioButton3;
        private System.Windows.Forms.RadioButton radioButton2;
        private System.Windows.Forms.RadioButton radioButton1;
    }
}