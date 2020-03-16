namespace MaintenanceToolGUI
{
    partial class DFUStartForm
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
            this.LabelUpdateVersion = new System.Windows.Forms.Label();
            this.LabelCurrentVersion = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.LabelComment = new System.Windows.Forms.Label();
            this.ButtonCancel = new System.Windows.Forms.Button();
            this.ButtonOK = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.LabelUpdateVersion);
            this.groupBox1.Controls.Add(this.LabelCurrentVersion);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(23, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(264, 82);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "バージョン情報";
            // 
            // LabelUpdateVersion
            // 
            this.LabelUpdateVersion.Location = new System.Drawing.Point(172, 51);
            this.LabelUpdateVersion.Name = "LabelUpdateVersion";
            this.LabelUpdateVersion.Size = new System.Drawing.Size(75, 12);
            this.LabelUpdateVersion.TabIndex = 13;
            this.LabelUpdateVersion.Text = "0.00.00";
            this.LabelUpdateVersion.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // LabelCurrentVersion
            // 
            this.LabelCurrentVersion.Location = new System.Drawing.Point(172, 25);
            this.LabelCurrentVersion.Name = "LabelCurrentVersion";
            this.LabelCurrentVersion.Size = new System.Drawing.Size(75, 12);
            this.LabelCurrentVersion.TabIndex = 12;
            this.LabelCurrentVersion.Text = "0.00.00";
            this.LabelCurrentVersion.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 51);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(150, 12);
            this.label1.TabIndex = 11;
            this.label1.Text = "更新するバージョン";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(16, 25);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(150, 12);
            this.label2.TabIndex = 10;
            this.label2.Text = "認証器に導入中のバージョン";
            // 
            // LabelComment
            // 
            this.LabelComment.Location = new System.Drawing.Point(23, 108);
            this.LabelComment.Name = "LabelComment";
            this.LabelComment.Size = new System.Drawing.Size(264, 79);
            this.LabelComment.TabIndex = 11;
            this.LabelComment.Text = "FIDO認証器をUSBポートに装着した状態で、\r\nRESETボタンを押すと、ブートローダーモードに遷移し、\r\n基板上の黄色・緑色LEDが連続点灯します。\r\n\r\nこ" +
    "の状態を確認したのち、OKボタンをクリックすると、\r\nファームウェア更新が開始されます。";
            this.LabelComment.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // ButtonCancel
            // 
            this.ButtonCancel.Location = new System.Drawing.Point(166, 203);
            this.ButtonCancel.Name = "ButtonCancel";
            this.ButtonCancel.Size = new System.Drawing.Size(75, 23);
            this.ButtonCancel.TabIndex = 12;
            this.ButtonCancel.Text = "Cancel";
            this.ButtonCancel.UseVisualStyleBackColor = true;
            this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
            // 
            // ButtonOK
            // 
            this.ButtonOK.Location = new System.Drawing.Point(68, 203);
            this.ButtonOK.Name = "ButtonOK";
            this.ButtonOK.Size = new System.Drawing.Size(75, 23);
            this.ButtonOK.TabIndex = 13;
            this.ButtonOK.Text = "OK";
            this.ButtonOK.UseVisualStyleBackColor = true;
            this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
            // 
            // DFUStartForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(309, 241);
            this.Controls.Add(this.ButtonCancel);
            this.Controls.Add(this.ButtonOK);
            this.Controls.Add(this.LabelComment);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DFUStartForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ファームウェアを更新";
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label LabelUpdateVersion;
        private System.Windows.Forms.Label LabelCurrentVersion;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label LabelComment;
        private System.Windows.Forms.Button ButtonCancel;
        private System.Windows.Forms.Button ButtonOK;
    }
}