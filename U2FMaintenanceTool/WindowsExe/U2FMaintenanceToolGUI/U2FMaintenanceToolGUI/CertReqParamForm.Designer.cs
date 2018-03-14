namespace U2FMaintenanceToolGUI
{
    partial class CertReqParamForm
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
            this.label2 = new System.Windows.Forms.Label();
            this.textPemPath = new System.Windows.Forms.TextBox();
            this.buttonPemPath = new System.Windows.Forms.Button();
            this.textCN = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.textOU = new System.Windows.Forms.TextBox();
            this.textO = new System.Windows.Forms.TextBox();
            this.textL = new System.Windows.Forms.TextBox();
            this.textST = new System.Windows.Forms.TextBox();
            this.textC = new System.Windows.Forms.TextBox();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(51, 12);
            this.label2.TabIndex = 8;
            this.label2.Text = "鍵ファイル";
            // 
            // textPemPath
            // 
            this.textPemPath.Location = new System.Drawing.Point(89, 12);
            this.textPemPath.Name = "textPemPath";
            this.textPemPath.Size = new System.Drawing.Size(427, 19);
            this.textPemPath.TabIndex = 1;
            // 
            // buttonPemPath
            // 
            this.buttonPemPath.Location = new System.Drawing.Point(522, 10);
            this.buttonPemPath.Name = "buttonPemPath";
            this.buttonPemPath.Size = new System.Drawing.Size(50, 23);
            this.buttonPemPath.TabIndex = 10;
            this.buttonPemPath.Text = "選択";
            this.buttonPemPath.UseVisualStyleBackColor = true;
            this.buttonPemPath.Click += new System.EventHandler(this.buttonPemPath_Click);
            // 
            // textCN
            // 
            this.textCN.Location = new System.Drawing.Point(89, 42);
            this.textCN.Name = "textCN";
            this.textCN.Size = new System.Drawing.Size(265, 19);
            this.textCN.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 45);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 12);
            this.label1.TabIndex = 12;
            this.label1.Text = "コモンネーム";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 75);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(65, 12);
            this.label3.TabIndex = 13;
            this.label3.Text = "組織単位名";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 105);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(41, 12);
            this.label4.TabIndex = 14;
            this.label4.Text = "組織名";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 135);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(53, 12);
            this.label5.TabIndex = 15;
            this.label5.Text = "市町村名";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 165);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(65, 12);
            this.label6.TabIndex = 16;
            this.label6.Text = "都道府県名";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(12, 195);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(29, 12);
            this.label7.TabIndex = 17;
            this.label7.Text = "国名";
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(307, 226);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 9;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // buttonOK
            // 
            this.buttonOK.Location = new System.Drawing.Point(197, 226);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 8;
            this.buttonOK.Text = "作成";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // textOU
            // 
            this.textOU.Location = new System.Drawing.Point(89, 72);
            this.textOU.Name = "textOU";
            this.textOU.Size = new System.Drawing.Size(265, 19);
            this.textOU.TabIndex = 3;
            // 
            // textO
            // 
            this.textO.Location = new System.Drawing.Point(89, 102);
            this.textO.Name = "textO";
            this.textO.Size = new System.Drawing.Size(265, 19);
            this.textO.TabIndex = 4;
            // 
            // textL
            // 
            this.textL.Location = new System.Drawing.Point(89, 132);
            this.textL.Name = "textL";
            this.textL.Size = new System.Drawing.Size(265, 19);
            this.textL.TabIndex = 5;
            // 
            // textST
            // 
            this.textST.Location = new System.Drawing.Point(89, 162);
            this.textST.Name = "textST";
            this.textST.Size = new System.Drawing.Size(265, 19);
            this.textST.TabIndex = 6;
            // 
            // textC
            // 
            this.textC.Location = new System.Drawing.Point(89, 192);
            this.textC.Name = "textC";
            this.textC.Size = new System.Drawing.Size(84, 19);
            this.textC.TabIndex = 7;
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // CertReqParamForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 261);
            this.Controls.Add(this.textC);
            this.Controls.Add(this.textST);
            this.Controls.Add(this.textL);
            this.Controls.Add(this.textO);
            this.Controls.Add(this.textOU);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textCN);
            this.Controls.Add(this.buttonPemPath);
            this.Controls.Add(this.textPemPath);
            this.Controls.Add(this.label2);
            this.Name = "CertReqParamForm";
            this.Text = "証明書要求ファイル(CSR)作成";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textPemPath;
        private System.Windows.Forms.Button buttonPemPath;
        private System.Windows.Forms.TextBox textCN;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.TextBox textOU;
        private System.Windows.Forms.TextBox textO;
        private System.Windows.Forms.TextBox textL;
        private System.Windows.Forms.TextBox textST;
        private System.Windows.Forms.TextBox textC;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
    }
}