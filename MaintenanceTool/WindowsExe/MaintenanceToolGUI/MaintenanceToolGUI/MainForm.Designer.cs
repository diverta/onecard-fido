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
            this.buttonSetPinParam = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
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
            this.ButtonFIDOAttestation = new System.Windows.Forms.Button();
            this.buttonSetPivParam = new System.Windows.Forms.Button();
            this.buttonDFU = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.buttonPairing = new System.Windows.Forms.Button();
            this.buttonUnpairing = new System.Windows.Forms.Button();
            this.buttonSetPgpParam = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonQuit
            // 
            this.buttonQuit.Location = new System.Drawing.Point(462, 27);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(110, 25);
            this.buttonQuit.TabIndex = 6;
            this.buttonQuit.Text = "終了";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // buttonSetPinParam
            // 
            this.buttonSetPinParam.Location = new System.Drawing.Point(238, 27);
            this.buttonSetPinParam.Name = "buttonSetPinParam";
            this.buttonSetPinParam.Size = new System.Drawing.Size(150, 25);
            this.buttonSetPinParam.TabIndex = 3;
            this.buttonSetPinParam.Text = "PINコード設定";
            this.buttonSetPinParam.UseVisualStyleBackColor = true;
            this.buttonSetPinParam.Click += new System.EventHandler(this.buttonSetPinParam_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 120);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(560, 254);
            this.textBox1.TabIndex = 8;
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
            this.ViewLogFileToolStripMenuItem});
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
            this.uSBToolStripMenuItem.Size = new System.Drawing.Size(95, 22);
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
            this.bLEToolStripMenuItem.Size = new System.Drawing.Size(95, 22);
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
            this.ToolPreferenceStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.ToolPreferenceStripMenuItem.Text = "ツール設定(&P)...";
            this.ToolPreferenceStripMenuItem.Click += new System.EventHandler(this.ToolPreferenceStripMenuItem_Click);
            // 
            // ButtonFIDOAttestation
            // 
            this.ButtonFIDOAttestation.Location = new System.Drawing.Point(422, 89);
            this.ButtonFIDOAttestation.Name = "ButtonFIDOAttestation";
            this.ButtonFIDOAttestation.Size = new System.Drawing.Size(150, 25);
            this.ButtonFIDOAttestation.TabIndex = 7;
            this.ButtonFIDOAttestation.Text = "FIDO鍵・証明書設定";
            this.ButtonFIDOAttestation.UseVisualStyleBackColor = true;
            this.ButtonFIDOAttestation.Click += new System.EventHandler(this.ButtonFIDOAttestation_Click);
            // 
            // buttonSetPivParam
            // 
            this.buttonSetPivParam.Enabled = false;
            this.buttonSetPivParam.Location = new System.Drawing.Point(238, 58);
            this.buttonSetPivParam.Name = "buttonSetPivParam";
            this.buttonSetPivParam.Size = new System.Drawing.Size(150, 25);
            this.buttonSetPivParam.TabIndex = 4;
            this.buttonSetPivParam.Text = "PIV機能設定";
            this.buttonSetPivParam.UseVisualStyleBackColor = true;
            // 
            // buttonDFU
            // 
            this.buttonDFU.Location = new System.Drawing.Point(238, 89);
            this.buttonDFU.Name = "buttonDFU";
            this.buttonDFU.Size = new System.Drawing.Size(150, 25);
            this.buttonDFU.TabIndex = 5;
            this.buttonDFU.Text = "ファームウェア更新";
            this.buttonDFU.UseVisualStyleBackColor = true;
            this.buttonDFU.Click += new System.EventHandler(this.buttonDFU_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.buttonPairing);
            this.groupBox1.Controls.Add(this.buttonUnpairing);
            this.groupBox1.Location = new System.Drawing.Point(12, 27);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(191, 87);
            this.groupBox1.TabIndex = 18;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "BLEペアリング";
            // 
            // buttonPairing
            // 
            this.buttonPairing.Location = new System.Drawing.Point(18, 18);
            this.buttonPairing.Name = "buttonPairing";
            this.buttonPairing.Size = new System.Drawing.Size(150, 25);
            this.buttonPairing.TabIndex = 1;
            this.buttonPairing.Text = "ペアリング実行";
            this.buttonPairing.UseVisualStyleBackColor = true;
            this.buttonPairing.Click += new System.EventHandler(this.buttonPairing_Click);
            // 
            // buttonUnpairing
            // 
            this.buttonUnpairing.Location = new System.Drawing.Point(18, 49);
            this.buttonUnpairing.Name = "buttonUnpairing";
            this.buttonUnpairing.Size = new System.Drawing.Size(150, 25);
            this.buttonUnpairing.TabIndex = 2;
            this.buttonUnpairing.Text = "ペアリング解除";
            this.buttonUnpairing.UseVisualStyleBackColor = true;
            this.buttonUnpairing.Click += new System.EventHandler(this.buttonUnpairing_Click);
            // 
            // buttonSetPgpParam
            // 
            this.buttonSetPgpParam.Location = new System.Drawing.Point(422, 58);
            this.buttonSetPgpParam.Name = "buttonSetPgpParam";
            this.buttonSetPgpParam.Size = new System.Drawing.Size(150, 25);
            this.buttonSetPgpParam.TabIndex = 19;
            this.buttonSetPgpParam.Text = "OpenPGP機能設定";
            this.buttonSetPgpParam.UseVisualStyleBackColor = true;
            this.buttonSetPgpParam.Click += new System.EventHandler(this.buttonSetPgpParam_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 386);
            this.Controls.Add(this.buttonSetPgpParam);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.buttonDFU);
            this.Controls.Add(this.buttonSetPivParam);
            this.Controls.Add(this.ButtonFIDOAttestation);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.buttonSetPinParam);
            this.Controls.Add(this.buttonQuit);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "MainForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonQuit;
        private System.Windows.Forms.Button buttonSetPinParam;
        private System.Windows.Forms.TextBox textBox1;
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
        private System.Windows.Forms.Button ButtonFIDOAttestation;
        private System.Windows.Forms.Button buttonSetPivParam;
        private System.Windows.Forms.Button buttonDFU;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonPairing;
        private System.Windows.Forms.Button buttonUnpairing;
        private System.Windows.Forms.Button buttonSetPgpParam;
    }
}

