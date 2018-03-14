namespace U2FMaintenanceToolGUI
{
    partial class SelfCertParamForm
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
            if (disposing && (components != null))
            {
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
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.textCsrPath = new System.Windows.Forms.TextBox();
            this.buttonCsrPath = new System.Windows.Forms.Button();
            this.buttonPemPath = new System.Windows.Forms.Button();
            this.textPemPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.textDays = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // buttonOK
            // 
            this.buttonOK.Location = new System.Drawing.Point(199, 117);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 5;
            this.buttonOK.Text = "作成";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(309, 117);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 6;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(99, 12);
            this.label1.TabIndex = 2;
            this.label1.Text = "証明書要求ファイル";
            // 
            // textCsrPath
            // 
            this.textCsrPath.Location = new System.Drawing.Point(117, 12);
            this.textCsrPath.Name = "textCsrPath";
            this.textCsrPath.Size = new System.Drawing.Size(399, 19);
            this.textCsrPath.TabIndex = 2;
            // 
            // buttonCsrPath
            // 
            this.buttonCsrPath.Location = new System.Drawing.Point(522, 10);
            this.buttonCsrPath.Name = "buttonCsrPath";
            this.buttonCsrPath.Size = new System.Drawing.Size(50, 23);
            this.buttonCsrPath.TabIndex = 7;
            this.buttonCsrPath.Text = "選択";
            this.buttonCsrPath.UseVisualStyleBackColor = true;
            this.buttonCsrPath.Click += new System.EventHandler(this.buttonCsrPath_Click);
            // 
            // buttonPemPath
            // 
            this.buttonPemPath.Location = new System.Drawing.Point(522, 41);
            this.buttonPemPath.Name = "buttonPemPath";
            this.buttonPemPath.Size = new System.Drawing.Size(50, 23);
            this.buttonPemPath.TabIndex = 8;
            this.buttonPemPath.Text = "選択";
            this.buttonPemPath.UseVisualStyleBackColor = true;
            this.buttonPemPath.Click += new System.EventHandler(this.buttonPemPath_Click);
            // 
            // textPemPath
            // 
            this.textPemPath.Location = new System.Drawing.Point(117, 43);
            this.textPemPath.Name = "textPemPath";
            this.textPemPath.Size = new System.Drawing.Size(399, 19);
            this.textPemPath.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 46);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 12);
            this.label2.TabIndex = 7;
            this.label2.Text = "秘密鍵ファイル";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 79);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(53, 12);
            this.label3.TabIndex = 8;
            this.label3.Text = "有効期間";
            // 
            // textDays
            // 
            this.textDays.Location = new System.Drawing.Point(117, 76);
            this.textDays.Name = "textDays";
            this.textDays.Size = new System.Drawing.Size(88, 19);
            this.textDays.TabIndex = 4;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(211, 79);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 12);
            this.label4.TabIndex = 10;
            this.label4.Text = "日間";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 152);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textDays);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textPemPath);
            this.Controls.Add(this.buttonPemPath);
            this.Controls.Add(this.buttonCsrPath);
            this.Controls.Add(this.textCsrPath);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Name = "Form1";
            this.Text = "自己署名証明書ファイル(CRT)作成";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textCsrPath;
        private System.Windows.Forms.Button buttonCsrPath;
        private System.Windows.Forms.Button buttonPemPath;
        private System.Windows.Forms.TextBox textPemPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textDays;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
    }
}