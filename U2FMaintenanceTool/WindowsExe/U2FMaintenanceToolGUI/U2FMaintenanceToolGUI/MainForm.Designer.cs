namespace U2FMaintenanceToolGUI
{
    partial class MainForm
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージ リソースを破棄する場合は true を指定し、その他の場合は false を指定します。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonQuit = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.textPath1 = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.textPath2 = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonPath1 = new System.Windows.Forms.Button();
            this.buttonPath2 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // buttonQuit
            // 
            this.buttonQuit.Location = new System.Drawing.Point(462, 12);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(110, 25);
            this.buttonQuit.TabIndex = 0;
            this.buttonQuit.Text = "終了";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(13, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(200, 25);
            this.button1.TabIndex = 1;
            this.button1.Text = "ペアリング情報消去";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(13, 43);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(200, 25);
            this.button2.TabIndex = 2;
            this.button2.Text = "鍵・証明書消去／AES暗号生成";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // textPath1
            // 
            this.textPath1.Location = new System.Drawing.Point(121, 77);
            this.textPath1.Name = "textPath1";
            this.textPath1.Size = new System.Drawing.Size(395, 19);
            this.textPath1.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(11, 80);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 12);
            this.label1.TabIndex = 4;
            this.label1.Text = "鍵ファイルのパス";
            // 
            // textPath2
            // 
            this.textPath2.Location = new System.Drawing.Point(121, 108);
            this.textPath2.Name = "textPath2";
            this.textPath2.Size = new System.Drawing.Size(395, 19);
            this.textPath2.TabIndex = 5;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(11, 111);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(104, 12);
            this.label2.TabIndex = 6;
            this.label2.Text = "証明書ファイルのパス";
            // 
            // buttonPath1
            // 
            this.buttonPath1.Location = new System.Drawing.Point(522, 74);
            this.buttonPath1.Name = "buttonPath1";
            this.buttonPath1.Size = new System.Drawing.Size(50, 25);
            this.buttonPath1.TabIndex = 7;
            this.buttonPath1.Text = "参照";
            this.buttonPath1.UseVisualStyleBackColor = true;
            this.buttonPath1.Click += new System.EventHandler(this.buttonPath1_Click);
            // 
            // buttonPath2
            // 
            this.buttonPath2.Location = new System.Drawing.Point(522, 105);
            this.buttonPath2.Name = "buttonPath2";
            this.buttonPath2.Size = new System.Drawing.Size(50, 25);
            this.buttonPath2.TabIndex = 8;
            this.buttonPath2.Text = "参照";
            this.buttonPath2.UseVisualStyleBackColor = true;
            this.buttonPath2.Click += new System.EventHandler(this.buttonPath2_Click);
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(12, 133);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(200, 25);
            this.button3.TabIndex = 9;
            this.button3.Text = "鍵・証明書ファイルのインストール";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(12, 164);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(200, 25);
            this.button4.TabIndex = 10;
            this.button4.Text = "ヘルスチェック実行";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(462, 164);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(110, 25);
            this.button5.TabIndex = 11;
            this.button5.Text = "Chrome設定";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.button5_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 195);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(560, 164);
            this.textBox1.TabIndex = 12;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 371);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.button5);
            this.Controls.Add(this.button4);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.buttonPath2);
            this.Controls.Add(this.buttonPath1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textPath2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textPath1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.buttonQuit);
            this.Name = "MainForm";
            this.Text = "U2F Maintenance Tool";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonQuit;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.TextBox textPath1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textPath2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonPath1;
        private System.Windows.Forms.Button buttonPath2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.TextBox textBox1;
    }
}

