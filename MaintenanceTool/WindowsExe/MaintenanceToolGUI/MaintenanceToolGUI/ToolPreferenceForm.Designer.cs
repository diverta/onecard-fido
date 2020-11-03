namespace MaintenanceToolGUI
{
    partial class ToolPreferenceForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ToolPreferenceForm));
            this.tabPreference = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.label3 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.checkNeedPairing = new System.Windows.Forms.CheckBox();
            this.textScanSec = new System.Windows.Forms.TextBox();
            this.textScanUUID = new System.Windows.Forms.TextBox();
            this.checkScanEnable = new System.Windows.Forms.CheckBox();
            this.buttonReset = new System.Windows.Forms.Button();
            this.buttonWrite = new System.Windows.Forms.Button();
            this.buttonRead = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.labelCopyright = new System.Windows.Forms.Label();
            this.labelVersion = new System.Windows.Forms.Label();
            this.labelToolName = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.tabPreference.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // tabPreference
            // 
            this.tabPreference.Controls.Add(this.tabPage1);
            this.tabPreference.Controls.Add(this.tabPage2);
            this.tabPreference.Location = new System.Drawing.Point(12, 12);
            this.tabPreference.Name = "tabPreference";
            this.tabPreference.SelectedIndex = 0;
            this.tabPreference.Size = new System.Drawing.Size(460, 236);
            this.tabPreference.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.label3);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Controls.Add(this.checkNeedPairing);
            this.tabPage1.Controls.Add(this.textScanSec);
            this.tabPage1.Controls.Add(this.textScanUUID);
            this.tabPage1.Controls.Add(this.checkScanEnable);
            this.tabPage1.Controls.Add(this.buttonReset);
            this.tabPage1.Controls.Add(this.buttonWrite);
            this.tabPage1.Controls.Add(this.buttonRead);
            this.tabPage1.Controls.Add(this.label5);
            this.tabPage1.Controls.Add(this.label2);
            this.tabPage1.Controls.Add(this.label4);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(452, 210);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "自動認証設定";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(203, 50);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(224, 36);
            this.label3.TabIndex = 13;
            this.label3.Text = "スマートフォンをBLEデバイスとして使用したい場合は、専用アプリをインストールした後、専用アプリによりペアリングを行ってください。";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(26, 54);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(150, 24);
            this.label1.TabIndex = 12;
            this.label1.Text = "スマートフォンをBLEデバイスとして使用";
            // 
            // checkNeedPairing
            // 
            this.checkNeedPairing.AutoSize = true;
            this.checkNeedPairing.Location = new System.Drawing.Point(182, 60);
            this.checkNeedPairing.Name = "checkNeedPairing";
            this.checkNeedPairing.Size = new System.Drawing.Size(15, 14);
            this.checkNeedPairing.TabIndex = 11;
            this.checkNeedPairing.UseVisualStyleBackColor = true;
            // 
            // textScanSec
            // 
            this.textScanSec.Location = new System.Drawing.Point(182, 126);
            this.textScanSec.Name = "textScanSec";
            this.textScanSec.Size = new System.Drawing.Size(40, 19);
            this.textScanSec.TabIndex = 4;
            // 
            // textScanUUID
            // 
            this.textScanUUID.Location = new System.Drawing.Point(182, 96);
            this.textScanUUID.Name = "textScanUUID";
            this.textScanUUID.Size = new System.Drawing.Size(245, 19);
            this.textScanUUID.TabIndex = 3;
            // 
            // checkScanEnable
            // 
            this.checkScanEnable.AutoSize = true;
            this.checkScanEnable.Location = new System.Drawing.Point(182, 24);
            this.checkScanEnable.Name = "checkScanEnable";
            this.checkScanEnable.Size = new System.Drawing.Size(15, 14);
            this.checkScanEnable.TabIndex = 2;
            this.checkScanEnable.UseVisualStyleBackColor = true;
            // 
            // buttonReset
            // 
            this.buttonReset.Location = new System.Drawing.Point(286, 164);
            this.buttonReset.Name = "buttonReset";
            this.buttonReset.Size = new System.Drawing.Size(80, 23);
            this.buttonReset.TabIndex = 7;
            this.buttonReset.Text = "設定解除";
            this.buttonReset.UseVisualStyleBackColor = true;
            this.buttonReset.Click += new System.EventHandler(this.buttonReset_Click);
            // 
            // buttonWrite
            // 
            this.buttonWrite.Location = new System.Drawing.Point(186, 164);
            this.buttonWrite.Name = "buttonWrite";
            this.buttonWrite.Size = new System.Drawing.Size(80, 23);
            this.buttonWrite.TabIndex = 6;
            this.buttonWrite.Text = "設定書込";
            this.buttonWrite.UseVisualStyleBackColor = true;
            this.buttonWrite.Click += new System.EventHandler(this.buttonWrite_Click);
            // 
            // buttonRead
            // 
            this.buttonRead.Location = new System.Drawing.Point(86, 164);
            this.buttonRead.Name = "buttonRead";
            this.buttonRead.Size = new System.Drawing.Size(80, 23);
            this.buttonRead.TabIndex = 5;
            this.buttonRead.Text = "設定読込";
            this.buttonRead.UseVisualStyleBackColor = true;
            this.buttonRead.Click += new System.EventHandler(this.buttonRead_Click);
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(26, 129);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(150, 12);
            this.label5.TabIndex = 10;
            this.label5.Text = "スキャン秒数";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(26, 99);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(150, 12);
            this.label2.TabIndex = 9;
            this.label2.Text = "スキャン対象サービスUUID";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(26, 25);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(150, 12);
            this.label4.TabIndex = 8;
            this.label4.Text = "自動認証機能を有効化";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.labelCopyright);
            this.tabPage2.Controls.Add(this.labelVersion);
            this.tabPage2.Controls.Add(this.labelToolName);
            this.tabPage2.Controls.Add(this.pictureBox1);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(452, 210);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "バージョン";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // labelCopyright
            // 
            this.labelCopyright.Location = new System.Drawing.Point(113, 122);
            this.labelCopyright.Name = "labelCopyright";
            this.labelCopyright.Size = new System.Drawing.Size(226, 12);
            this.labelCopyright.TabIndex = 7;
            this.labelCopyright.Text = "labelCopywrite";
            this.labelCopyright.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelVersion
            // 
            this.labelVersion.Location = new System.Drawing.Point(176, 86);
            this.labelVersion.Name = "labelVersion";
            this.labelVersion.Size = new System.Drawing.Size(150, 12);
            this.labelVersion.TabIndex = 6;
            this.labelVersion.Text = "labelVersion";
            // 
            // labelToolName
            // 
            this.labelToolName.Location = new System.Drawing.Point(176, 54);
            this.labelToolName.Name = "labelToolName";
            this.labelToolName.Size = new System.Drawing.Size(150, 12);
            this.labelToolName.TabIndex = 5;
            this.labelToolName.Text = "labelToolName";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(78, 34);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(64, 64);
            this.pictureBox1.TabIndex = 4;
            this.pictureBox1.TabStop = false;
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(202, 263);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(80, 23);
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "閉じる";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // ToolPreferenceForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 304);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.tabPreference);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ToolPreferenceForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ツール設定";
            this.tabPreference.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabPreference;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label labelCopyright;
        private System.Windows.Forms.Label labelVersion;
        private System.Windows.Forms.Label labelToolName;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.CheckBox checkScanEnable;
        private System.Windows.Forms.Button buttonReset;
        private System.Windows.Forms.Button buttonWrite;
        private System.Windows.Forms.Button buttonRead;
        private System.Windows.Forms.TextBox textScanSec;
        private System.Windows.Forms.TextBox textScanUUID;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox checkNeedPairing;
    }
}