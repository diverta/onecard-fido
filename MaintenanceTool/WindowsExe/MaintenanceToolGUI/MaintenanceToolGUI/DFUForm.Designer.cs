namespace MaintenanceToolGUI
{
    partial class DFUForm
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.buttonUSBDFU = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.buttonBLEDFU = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(152, 297);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(80, 23);
            this.buttonCancel.TabIndex = 3;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.buttonUSBDFU);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(360, 127);
            this.groupBox1.TabIndex = 11;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "nRF52840用";
            // 
            // buttonUSBDFU
            // 
            this.buttonUSBDFU.Location = new System.Drawing.Point(8, 18);
            this.buttonUSBDFU.Name = "buttonUSBDFU";
            this.buttonUSBDFU.Size = new System.Drawing.Size(346, 23);
            this.buttonUSBDFU.TabIndex = 1;
            this.buttonUSBDFU.Text = "ファームウェアを更新（USB）";
            this.buttonUSBDFU.UseVisualStyleBackColor = true;
            this.buttonUSBDFU.Click += new System.EventHandler(this.buttonUSBDFU_Click);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(6, 55);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(348, 60);
            this.label3.TabIndex = 11;
            this.label3.Text = "USB経由でファームウェアを更新します。\r\n\r\nFIDO認証器は、バージョン0.3.0以降のファームウェアが導入済みのものをご利用ください。";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.buttonBLEDFU);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Location = new System.Drawing.Point(12, 154);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(360, 127);
            this.groupBox2.TabIndex = 12;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "nRF5340用";
            // 
            // buttonBLEDFU
            // 
            this.buttonBLEDFU.Enabled = false;
            this.buttonBLEDFU.Location = new System.Drawing.Point(8, 18);
            this.buttonBLEDFU.Name = "buttonBLEDFU";
            this.buttonBLEDFU.Size = new System.Drawing.Size(346, 23);
            this.buttonBLEDFU.TabIndex = 2;
            this.buttonBLEDFU.Text = "ファームウェアを更新（BLE）";
            this.buttonBLEDFU.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(6, 55);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(348, 60);
            this.label1.TabIndex = 11;
            this.label1.Text = "BLE経由でファームウェアを更新します。\r\n\r\nFIDO認証器は、バージョン0.4.0以降のファームウェアが導入済みのものをご利用ください。";
            // 
            // DFUForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 332);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DFUForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ファームウェア更新";
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonUSBDFU;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button buttonBLEDFU;
        private System.Windows.Forms.Label label1;
    }
}