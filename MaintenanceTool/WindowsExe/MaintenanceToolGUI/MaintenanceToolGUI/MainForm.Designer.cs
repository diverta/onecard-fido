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
            this.ButtonFIDOAttestation = new System.Windows.Forms.Button();
            this.buttonSetPivParam = new System.Windows.Forms.Button();
            this.buttonDFU = new System.Windows.Forms.Button();
            this.buttonSetPgpParam = new System.Windows.Forms.Button();
            this.buttonBLE = new System.Windows.Forms.Button();
            this.buttonFIDO = new System.Windows.Forms.Button();
            this.buttonOATH = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonQuit
            // 
            this.buttonQuit.Location = new System.Drawing.Point(462, 12);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(110, 25);
            this.buttonQuit.TabIndex = 7;
            this.buttonQuit.Text = "終了";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // buttonSetPinParam
            // 
            this.buttonSetPinParam.Location = new System.Drawing.Point(393, 81);
            this.buttonSetPinParam.Name = "buttonSetPinParam";
            this.buttonSetPinParam.Size = new System.Drawing.Size(180, 25);
            this.buttonSetPinParam.TabIndex = 9;
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
            this.textBox1.TabIndex = 10;
            // 
            // ButtonFIDOAttestation
            // 
            this.ButtonFIDOAttestation.Location = new System.Drawing.Point(394, 46);
            this.ButtonFIDOAttestation.Name = "ButtonFIDOAttestation";
            this.ButtonFIDOAttestation.Size = new System.Drawing.Size(179, 25);
            this.ButtonFIDOAttestation.TabIndex = 8;
            this.ButtonFIDOAttestation.Text = "FIDO鍵・証明書設定";
            this.ButtonFIDOAttestation.UseVisualStyleBackColor = true;
            this.ButtonFIDOAttestation.Click += new System.EventHandler(this.ButtonFIDOAttestation_Click);
            // 
            // buttonSetPivParam
            // 
            this.buttonSetPivParam.Enabled = false;
            this.buttonSetPivParam.Location = new System.Drawing.Point(203, 12);
            this.buttonSetPivParam.Name = "buttonSetPivParam";
            this.buttonSetPivParam.Size = new System.Drawing.Size(180, 25);
            this.buttonSetPivParam.TabIndex = 4;
            this.buttonSetPivParam.Text = "PIV設定";
            this.buttonSetPivParam.UseVisualStyleBackColor = true;
            // 
            // buttonDFU
            // 
            this.buttonDFU.Location = new System.Drawing.Point(12, 81);
            this.buttonDFU.Name = "buttonDFU";
            this.buttonDFU.Size = new System.Drawing.Size(180, 25);
            this.buttonDFU.TabIndex = 3;
            this.buttonDFU.Text = "ファームウェア更新";
            this.buttonDFU.UseVisualStyleBackColor = true;
            this.buttonDFU.Click += new System.EventHandler(this.buttonDFU_Click);
            // 
            // buttonSetPgpParam
            // 
            this.buttonSetPgpParam.Location = new System.Drawing.Point(203, 46);
            this.buttonSetPgpParam.Name = "buttonSetPgpParam";
            this.buttonSetPgpParam.Size = new System.Drawing.Size(180, 25);
            this.buttonSetPgpParam.TabIndex = 5;
            this.buttonSetPgpParam.Text = "OpenPGP設定";
            this.buttonSetPgpParam.UseVisualStyleBackColor = true;
            this.buttonSetPgpParam.Click += new System.EventHandler(this.buttonSetPgpParam_Click);
            // 
            // buttonBLE
            // 
            this.buttonBLE.Location = new System.Drawing.Point(12, 12);
            this.buttonBLE.Name = "buttonBLE";
            this.buttonBLE.Size = new System.Drawing.Size(180, 25);
            this.buttonBLE.TabIndex = 1;
            this.buttonBLE.Text = "BLE設定";
            this.buttonBLE.UseVisualStyleBackColor = true;
            this.buttonBLE.Click += new System.EventHandler(this.buttonBLE_Click);
            // 
            // buttonFIDO
            // 
            this.buttonFIDO.Location = new System.Drawing.Point(12, 46);
            this.buttonFIDO.Name = "buttonFIDO";
            this.buttonFIDO.Size = new System.Drawing.Size(180, 25);
            this.buttonFIDO.TabIndex = 2;
            this.buttonFIDO.Text = "FIDO設定";
            this.buttonFIDO.UseVisualStyleBackColor = true;
            // 
            // buttonOATH
            // 
            this.buttonOATH.Enabled = false;
            this.buttonOATH.Location = new System.Drawing.Point(203, 81);
            this.buttonOATH.Name = "buttonOATH";
            this.buttonOATH.Size = new System.Drawing.Size(180, 25);
            this.buttonOATH.TabIndex = 6;
            this.buttonOATH.Text = "OATH設定";
            this.buttonOATH.UseVisualStyleBackColor = true;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 386);
            this.Controls.Add(this.buttonOATH);
            this.Controls.Add(this.buttonFIDO);
            this.Controls.Add(this.buttonBLE);
            this.Controls.Add(this.buttonSetPgpParam);
            this.Controls.Add(this.buttonDFU);
            this.Controls.Add(this.buttonSetPivParam);
            this.Controls.Add(this.ButtonFIDOAttestation);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.buttonSetPinParam);
            this.Controls.Add(this.buttonQuit);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "MainForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonQuit;
        private System.Windows.Forms.Button buttonSetPinParam;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button ButtonFIDOAttestation;
        private System.Windows.Forms.Button buttonSetPivParam;
        private System.Windows.Forms.Button buttonDFU;
        private System.Windows.Forms.Button buttonSetPgpParam;
        private System.Windows.Forms.Button buttonBLE;
        private System.Windows.Forms.Button buttonFIDO;
        private System.Windows.Forms.Button buttonOATH;
    }
}

