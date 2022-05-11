namespace DevelopmentToolGUI
{
    partial class UtilityForm
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
            this.buttonToolVersionInfo = new System.Windows.Forms.Button();
            this.buttonViewLogFile = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(107, 120);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(79, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // buttonToolVersionInfo
            // 
            this.buttonToolVersionInfo.Location = new System.Drawing.Point(56, 20);
            this.buttonToolVersionInfo.Name = "buttonToolVersionInfo";
            this.buttonToolVersionInfo.Size = new System.Drawing.Size(180, 25);
            this.buttonToolVersionInfo.TabIndex = 3;
            this.buttonToolVersionInfo.Text = "開発ツールのバージョンを参照";
            this.buttonToolVersionInfo.UseVisualStyleBackColor = true;
            this.buttonToolVersionInfo.Click += new System.EventHandler(this.buttonToolVersionInfo_Click);
            // 
            // buttonViewLogFile
            // 
            this.buttonViewLogFile.Location = new System.Drawing.Point(56, 70);
            this.buttonViewLogFile.Name = "buttonViewLogFile";
            this.buttonViewLogFile.Size = new System.Drawing.Size(180, 25);
            this.buttonViewLogFile.TabIndex = 4;
            this.buttonViewLogFile.Text = "開発ツールのログを参照";
            this.buttonViewLogFile.UseVisualStyleBackColor = true;
            this.buttonViewLogFile.Click += new System.EventHandler(this.buttonViewLogFile_Click);
            // 
            // UtilityForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 162);
            this.Controls.Add(this.buttonViewLogFile);
            this.Controls.Add(this.buttonToolVersionInfo);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UtilityForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ユーティリティー";
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonToolVersionInfo;
        private System.Windows.Forms.Button buttonViewLogFile;
    }
}