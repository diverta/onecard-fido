namespace MaintenanceToolGUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
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
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.ファイルFToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ViewLogFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.テストTToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.uSBToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoHIDCtap2TestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoHIDU2fTestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoHIDPingTestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoHIDGetFlashInfoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoHIDGetVersionInfoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.bLEToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoBLECtap2TestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoBLEU2fTestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.DoBLEPingTestToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.その他OToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolPreferenceStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.DFUToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonQuit
            // 
            this.buttonQuit.Location = new System.Drawing.Point(462, 27);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(110, 25);
            this.buttonQuit.TabIndex = 0;
            this.buttonQuit.Text = "終了";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(13, 27);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(200, 25);
            this.button1.TabIndex = 1;
            this.button1.Text = "ペアリング実行";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(13, 58);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(200, 25);
            this.button2.TabIndex = 2;
            this.button2.Text = "鍵・証明書・キーハンドル消去";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // textPath1
            // 
            this.textPath1.Location = new System.Drawing.Point(121, 92);
            this.textPath1.Name = "textPath1";
            this.textPath1.Size = new System.Drawing.Size(395, 19);
            this.textPath1.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(11, 95);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 12);
            this.label1.TabIndex = 4;
            this.label1.Text = "鍵ファイルのパス";
            // 
            // textPath2
            // 
            this.textPath2.Location = new System.Drawing.Point(121, 123);
            this.textPath2.Name = "textPath2";
            this.textPath2.Size = new System.Drawing.Size(395, 19);
            this.textPath2.TabIndex = 5;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(11, 126);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(104, 12);
            this.label2.TabIndex = 6;
            this.label2.Text = "証明書ファイルのパス";
            // 
            // buttonPath1
            // 
            this.buttonPath1.Location = new System.Drawing.Point(522, 89);
            this.buttonPath1.Name = "buttonPath1";
            this.buttonPath1.Size = new System.Drawing.Size(50, 25);
            this.buttonPath1.TabIndex = 7;
            this.buttonPath1.Text = "参照";
            this.buttonPath1.UseVisualStyleBackColor = true;
            this.buttonPath1.Click += new System.EventHandler(this.buttonPath1_Click);
            // 
            // buttonPath2
            // 
            this.buttonPath2.Location = new System.Drawing.Point(522, 120);
            this.buttonPath2.Name = "buttonPath2";
            this.buttonPath2.Size = new System.Drawing.Size(50, 25);
            this.buttonPath2.TabIndex = 8;
            this.buttonPath2.Text = "参照";
            this.buttonPath2.UseVisualStyleBackColor = true;
            this.buttonPath2.Click += new System.EventHandler(this.buttonPath2_Click);
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(12, 148);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(200, 25);
            this.button3.TabIndex = 9;
            this.button3.Text = "鍵・証明書ファイルのインストール";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(12, 179);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(200, 25);
            this.button4.TabIndex = 10;
            this.button4.Text = "PINコード設定";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 210);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(560, 164);
            this.textBox1.TabIndex = 12;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ファイルFToolStripMenuItem,
            this.テストTToolStripMenuItem,
            this.その他OToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(584, 24);
            this.menuStrip1.TabIndex = 13;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // ファイルFToolStripMenuItem
            // 
            this.ファイルFToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ViewLogFileToolStripMenuItem,
            this.DFUToolStripMenuItem});
            this.ファイルFToolStripMenuItem.Name = "ファイルFToolStripMenuItem";
            this.ファイルFToolStripMenuItem.Size = new System.Drawing.Size(67, 20);
            this.ファイルFToolStripMenuItem.Text = "ファイル(&F)";
            // 
            // ViewLogFileToolStripMenuItem
            // 
            this.ViewLogFileToolStripMenuItem.Name = "ViewLogFileToolStripMenuItem";
            this.ViewLogFileToolStripMenuItem.Size = new System.Drawing.Size(200, 22);
            this.ViewLogFileToolStripMenuItem.Text = "管理ツールのログを参照(&L)";
            this.ViewLogFileToolStripMenuItem.Click += new System.EventHandler(this.ViewLogFileToolStripMenuItem_Click);
            // 
            // テストTToolStripMenuItem
            // 
            this.テストTToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.uSBToolStripMenuItem,
            this.bLEToolStripMenuItem});
            this.テストTToolStripMenuItem.Name = "テストTToolStripMenuItem";
            this.テストTToolStripMenuItem.Size = new System.Drawing.Size(59, 20);
            this.テストTToolStripMenuItem.Text = "テスト(&T)";
            // 
            // uSBToolStripMenuItem
            // 
            this.uSBToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.DoHIDCtap2TestToolStripMenuItem,
            this.DoHIDU2fTestToolStripMenuItem,
            this.DoHIDPingTestToolStripMenuItem,
            this.DoHIDGetFlashInfoToolStripMenuItem,
            this.DoHIDGetVersionInfoToolStripMenuItem});
            this.uSBToolStripMenuItem.Name = "uSBToolStripMenuItem";
            this.uSBToolStripMenuItem.Size = new System.Drawing.Size(180, 22);
            this.uSBToolStripMenuItem.Text = "USB";
            // 
            // DoHIDCtap2TestToolStripMenuItem
            // 
            this.DoHIDCtap2TestToolStripMenuItem.Name = "DoHIDCtap2TestToolStripMenuItem";
            this.DoHIDCtap2TestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoHIDCtap2TestToolStripMenuItem.Text = "CTAP2ヘルスチェック実行";
            this.DoHIDCtap2TestToolStripMenuItem.Click += new System.EventHandler(this.DoHIDCtap2TestToolStripMenuItem_Click);
            // 
            // DoHIDU2fTestToolStripMenuItem
            // 
            this.DoHIDU2fTestToolStripMenuItem.Name = "DoHIDU2fTestToolStripMenuItem";
            this.DoHIDU2fTestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoHIDU2fTestToolStripMenuItem.Text = "U2Fヘルスチェック実行";
            this.DoHIDU2fTestToolStripMenuItem.Click += new System.EventHandler(this.DoHIDU2fTestToolStripMenuItem_Click);
            // 
            // DoHIDPingTestToolStripMenuItem
            // 
            this.DoHIDPingTestToolStripMenuItem.Name = "DoHIDPingTestToolStripMenuItem";
            this.DoHIDPingTestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoHIDPingTestToolStripMenuItem.Text = "PINGテスト実行";
            this.DoHIDPingTestToolStripMenuItem.Click += new System.EventHandler(this.DoHIDPingTestToolStripMenuItem_Click);
            // 
            // DoHIDGetFlashInfoToolStripMenuItem
            // 
            this.DoHIDGetFlashInfoToolStripMenuItem.Name = "DoHIDGetFlashInfoToolStripMenuItem";
            this.DoHIDGetFlashInfoToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoHIDGetFlashInfoToolStripMenuItem.Text = "Flash ROM情報取得";
            this.DoHIDGetFlashInfoToolStripMenuItem.Click += new System.EventHandler(this.DoHIDGetFlashInfoToolStripMenuItem_Click);
            // 
            // DoHIDGetVersionInfoToolStripMenuItem
            // 
            this.DoHIDGetVersionInfoToolStripMenuItem.Name = "DoHIDGetVersionInfoToolStripMenuItem";
            this.DoHIDGetVersionInfoToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoHIDGetVersionInfoToolStripMenuItem.Text = "バージョン情報取得";
            this.DoHIDGetVersionInfoToolStripMenuItem.Click += new System.EventHandler(this.DoHIDGetVersionInfoToolStripMenuItem_Click);
            // 
            // bLEToolStripMenuItem
            // 
            this.bLEToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.DoBLECtap2TestToolStripMenuItem,
            this.DoBLEU2fTestToolStripMenuItem,
            this.DoBLEPingTestToolStripMenuItem});
            this.bLEToolStripMenuItem.Name = "bLEToolStripMenuItem";
            this.bLEToolStripMenuItem.Size = new System.Drawing.Size(180, 22);
            this.bLEToolStripMenuItem.Text = "BLE";
            // 
            // DoBLECtap2TestToolStripMenuItem
            // 
            this.DoBLECtap2TestToolStripMenuItem.Name = "DoBLECtap2TestToolStripMenuItem";
            this.DoBLECtap2TestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoBLECtap2TestToolStripMenuItem.Text = "CTAP2ヘルスチェック実行";
            this.DoBLECtap2TestToolStripMenuItem.Click += new System.EventHandler(this.DoBLECtap2TestToolStripMenuItem_Click);
            // 
            // DoBLEU2fTestToolStripMenuItem
            // 
            this.DoBLEU2fTestToolStripMenuItem.Name = "DoBLEU2fTestToolStripMenuItem";
            this.DoBLEU2fTestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoBLEU2fTestToolStripMenuItem.Text = "U2Fヘルスチェック実行";
            this.DoBLEU2fTestToolStripMenuItem.Click += new System.EventHandler(this.DoBLEU2fTestToolStripMenuItem_Click);
            // 
            // DoBLEPingTestToolStripMenuItem
            // 
            this.DoBLEPingTestToolStripMenuItem.Name = "DoBLEPingTestToolStripMenuItem";
            this.DoBLEPingTestToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.DoBLEPingTestToolStripMenuItem.Text = "PINGテスト実行";
            this.DoBLEPingTestToolStripMenuItem.Click += new System.EventHandler(this.DoBLEPingCommandToolStripMenuItem_Click);
            // 
            // その他OToolStripMenuItem
            // 
            this.その他OToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolPreferenceStripMenuItem});
            this.その他OToolStripMenuItem.Name = "その他OToolStripMenuItem";
            this.その他OToolStripMenuItem.Size = new System.Drawing.Size(67, 20);
            this.その他OToolStripMenuItem.Text = "その他(&O)";
            // 
            // ToolPreferenceStripMenuItem
            // 
            this.ToolPreferenceStripMenuItem.Name = "ToolPreferenceStripMenuItem";
            this.ToolPreferenceStripMenuItem.Size = new System.Drawing.Size(180, 22);
            this.ToolPreferenceStripMenuItem.Text = "ツール設定(&P)...";
            this.ToolPreferenceStripMenuItem.Click += new System.EventHandler(this.ToolPreferenceStripMenuItem_Click);
            // 
            // DFUToolStripMenuItem
            // 
            this.DFUToolStripMenuItem.Name = "DFUToolStripMenuItem";
            this.DFUToolStripMenuItem.Size = new System.Drawing.Size(200, 22);
            this.DFUToolStripMenuItem.Text = "ファームウェアを更新";
            this.DFUToolStripMenuItem.Click += new System.EventHandler(this.DFUToolStripMenuItem_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 386);
            this.Controls.Add(this.textBox1);
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
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "MainForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
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
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.ToolStripMenuItem その他OToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolPreferenceStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem テストTToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem uSBToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoHIDPingTestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem bLEToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoBLEU2fTestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoHIDCtap2TestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoHIDGetFlashInfoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoBLEPingTestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoBLECtap2TestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoHIDU2fTestToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DoHIDGetVersionInfoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ファイルFToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ViewLogFileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem DFUToolStripMenuItem;
    }
}

