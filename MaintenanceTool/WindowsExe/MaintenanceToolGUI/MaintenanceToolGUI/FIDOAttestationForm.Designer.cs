namespace MaintenanceToolGUI
{
    partial class FIDOAttestationForm
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
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.textKeyPath = new System.Windows.Forms.TextBox();
            this.textCertPath = new System.Windows.Forms.TextBox();
            this.buttonSelectKeyPath = new System.Windows.Forms.Button();
            this.buttonSelectCertPath = new System.Windows.Forms.Button();
            this.buttonInstall = new System.Windows.Forms.Button();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(497, 88);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 25);
            this.buttonCancel.TabIndex = 7;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 23);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 12);
            this.label1.TabIndex = 8;
            this.label1.Text = "鍵ファイルのパス";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(104, 12);
            this.label2.TabIndex = 9;
            this.label2.Text = "証明書ファイルのパス";
            // 
            // textKeyPath
            // 
            this.textKeyPath.Location = new System.Drawing.Point(121, 20);
            this.textKeyPath.Name = "textKeyPath";
            this.textKeyPath.ReadOnly = true;
            this.textKeyPath.Size = new System.Drawing.Size(395, 19);
            this.textKeyPath.TabIndex = 1;
            // 
            // textCertPath
            // 
            this.textCertPath.Location = new System.Drawing.Point(121, 51);
            this.textCertPath.Name = "textCertPath";
            this.textCertPath.ReadOnly = true;
            this.textCertPath.Size = new System.Drawing.Size(395, 19);
            this.textCertPath.TabIndex = 3;
            // 
            // buttonSelectKeyPath
            // 
            this.buttonSelectKeyPath.Location = new System.Drawing.Point(523, 17);
            this.buttonSelectKeyPath.Name = "buttonSelectKeyPath";
            this.buttonSelectKeyPath.Size = new System.Drawing.Size(50, 25);
            this.buttonSelectKeyPath.TabIndex = 2;
            this.buttonSelectKeyPath.Text = "参照";
            this.buttonSelectKeyPath.UseVisualStyleBackColor = true;
            this.buttonSelectKeyPath.Click += new System.EventHandler(this.buttonSelectKeyPath_Click);
            // 
            // buttonSelectCertPath
            // 
            this.buttonSelectCertPath.Location = new System.Drawing.Point(522, 48);
            this.buttonSelectCertPath.Name = "buttonSelectCertPath";
            this.buttonSelectCertPath.Size = new System.Drawing.Size(50, 25);
            this.buttonSelectCertPath.TabIndex = 4;
            this.buttonSelectCertPath.Text = "参照";
            this.buttonSelectCertPath.UseVisualStyleBackColor = true;
            this.buttonSelectCertPath.Click += new System.EventHandler(this.buttonSelectCertPath_Click);
            // 
            // buttonInstall
            // 
            this.buttonInstall.Location = new System.Drawing.Point(14, 88);
            this.buttonInstall.Name = "buttonInstall";
            this.buttonInstall.Size = new System.Drawing.Size(200, 25);
            this.buttonInstall.TabIndex = 5;
            this.buttonInstall.Text = "鍵・証明書ファイルのインストール";
            this.buttonInstall.UseVisualStyleBackColor = true;
            this.buttonInstall.Click += new System.EventHandler(this.buttonInstall_Click);
            // 
            // buttonDelete
            // 
            this.buttonDelete.Location = new System.Drawing.Point(232, 88);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(200, 25);
            this.buttonDelete.TabIndex = 6;
            this.buttonDelete.Text = "鍵・証明書の消去";
            this.buttonDelete.UseVisualStyleBackColor = true;
            this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
            // 
            // FIDOAttestationForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 131);
            this.Controls.Add(this.buttonDelete);
            this.Controls.Add(this.buttonInstall);
            this.Controls.Add(this.buttonSelectCertPath);
            this.Controls.Add(this.buttonSelectKeyPath);
            this.Controls.Add(this.textCertPath);
            this.Controls.Add(this.textKeyPath);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FIDOAttestationForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "FIDO鍵・証明書設定";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textKeyPath;
        private System.Windows.Forms.TextBox textCertPath;
        private System.Windows.Forms.Button buttonSelectKeyPath;
        private System.Windows.Forms.Button buttonSelectCertPath;
        private System.Windows.Forms.Button buttonInstall;
        private System.Windows.Forms.Button buttonDelete;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
    }
}