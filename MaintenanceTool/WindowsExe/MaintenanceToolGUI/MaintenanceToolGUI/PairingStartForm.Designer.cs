namespace MaintenanceToolGUI
{
    partial class PairingStartForm
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ButtonOK_2 = new System.Windows.Forms.Button();
            this.textPasskey = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.ButtonCancel = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ButtonOK_2);
            this.groupBox1.Controls.Add(this.textPasskey);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(12, 75);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(285, 93);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "ペアリング実行";
            // 
            // ButtonOK_2
            // 
            this.ButtonOK_2.Location = new System.Drawing.Point(57, 53);
            this.ButtonOK_2.Name = "ButtonOK_2";
            this.ButtonOK_2.Size = new System.Drawing.Size(169, 23);
            this.ButtonOK_2.TabIndex = 14;
            this.ButtonOK_2.Text = "ペアリングを実行する";
            this.ButtonOK_2.UseVisualStyleBackColor = true;
            this.ButtonOK_2.Click += new System.EventHandler(this.ButtonOK_2_Click);
            // 
            // textPasskey
            // 
            this.textPasskey.Location = new System.Drawing.Point(161, 24);
            this.textPasskey.MaxLength = 6;
            this.textPasskey.Name = "textPasskey";
            this.textPasskey.Size = new System.Drawing.Size(108, 19);
            this.textPasskey.TabIndex = 11;
            this.textPasskey.UseSystemPasswordChar = true;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(13, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(137, 20);
            this.label2.TabIndex = 10;
            this.label2.Text = "パスコード（半角数字6桁）";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // ButtonCancel
            // 
            this.ButtonCancel.Location = new System.Drawing.Point(116, 180);
            this.ButtonCancel.Name = "ButtonCancel";
            this.ButtonCancel.Size = new System.Drawing.Size(75, 23);
            this.ButtonCancel.TabIndex = 12;
            this.ButtonCancel.Text = "Cancel";
            this.ButtonCancel.UseVisualStyleBackColor = true;
            this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(12, 9);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(285, 63);
            this.label3.TabIndex = 16;
            this.label3.Text = "FIDO認証器とペアリングする場合は、\r\n基板背面に記載されたパスコードを入力し、\r\n下のボタンをクリックしてください。";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // PairingStartForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(309, 217);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.ButtonCancel);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PairingStartForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ペアリング実行";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button ButtonCancel;
        private System.Windows.Forms.TextBox textPasskey;
        private System.Windows.Forms.Button ButtonOK_2;
        private System.Windows.Forms.Label label3;
    }
}