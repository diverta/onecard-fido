namespace MaintenanceToolGUI
{
    partial class HealthCheckForm
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
            this.buttonBLEPingTest = new System.Windows.Forms.Button();
            this.buttonBLECtap2HealthCheck = new System.Windows.Forms.Button();
            this.buttonBLEU2FHealthCheck = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.buttonHIDPingTest = new System.Windows.Forms.Button();
            this.buttonHIDCtap2HealthCheck = new System.Windows.Forms.Button();
            this.buttonHIDU2FHealthCheck = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(108, 315);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(79, 23);
            this.buttonCancel.TabIndex = 7;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.buttonBLEPingTest);
            this.groupBox1.Controls.Add(this.buttonBLECtap2HealthCheck);
            this.groupBox1.Controls.Add(this.buttonBLEU2FHealthCheck);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(268, 138);
            this.groupBox1.TabIndex = 19;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "BLE";
            // 
            // buttonBLEPingTest
            // 
            this.buttonBLEPingTest.Location = new System.Drawing.Point(21, 98);
            this.buttonBLEPingTest.Name = "buttonBLEPingTest";
            this.buttonBLEPingTest.Size = new System.Drawing.Size(229, 25);
            this.buttonBLEPingTest.TabIndex = 3;
            this.buttonBLEPingTest.Text = "PINGテスト実行";
            this.buttonBLEPingTest.UseVisualStyleBackColor = true;
            this.buttonBLEPingTest.Click += new System.EventHandler(this.buttonBLEPingTest_Click);
            // 
            // buttonBLECtap2HealthCheck
            // 
            this.buttonBLECtap2HealthCheck.Location = new System.Drawing.Point(21, 24);
            this.buttonBLECtap2HealthCheck.Name = "buttonBLECtap2HealthCheck";
            this.buttonBLECtap2HealthCheck.Size = new System.Drawing.Size(229, 25);
            this.buttonBLECtap2HealthCheck.TabIndex = 1;
            this.buttonBLECtap2HealthCheck.Text = "CTAP2ヘルスチェック実行";
            this.buttonBLECtap2HealthCheck.UseVisualStyleBackColor = true;
            this.buttonBLECtap2HealthCheck.Click += new System.EventHandler(this.buttonBLECtap2HealthCheck_Click);
            // 
            // buttonBLEU2FHealthCheck
            // 
            this.buttonBLEU2FHealthCheck.Location = new System.Drawing.Point(21, 61);
            this.buttonBLEU2FHealthCheck.Name = "buttonBLEU2FHealthCheck";
            this.buttonBLEU2FHealthCheck.Size = new System.Drawing.Size(229, 25);
            this.buttonBLEU2FHealthCheck.TabIndex = 2;
            this.buttonBLEU2FHealthCheck.Text = "U2Fヘルスチェック実行";
            this.buttonBLEU2FHealthCheck.UseVisualStyleBackColor = true;
            this.buttonBLEU2FHealthCheck.Click += new System.EventHandler(this.buttonBLEU2FHealthCheck_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.buttonHIDPingTest);
            this.groupBox2.Controls.Add(this.buttonHIDCtap2HealthCheck);
            this.groupBox2.Controls.Add(this.buttonHIDU2FHealthCheck);
            this.groupBox2.Location = new System.Drawing.Point(12, 161);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(268, 138);
            this.groupBox2.TabIndex = 20;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "USB";
            // 
            // buttonHIDPingTest
            // 
            this.buttonHIDPingTest.Location = new System.Drawing.Point(21, 98);
            this.buttonHIDPingTest.Name = "buttonHIDPingTest";
            this.buttonHIDPingTest.Size = new System.Drawing.Size(229, 25);
            this.buttonHIDPingTest.TabIndex = 6;
            this.buttonHIDPingTest.Text = "PINGテスト実行";
            this.buttonHIDPingTest.UseVisualStyleBackColor = true;
            this.buttonHIDPingTest.Click += new System.EventHandler(this.buttonHIDPingTest_Click);
            // 
            // buttonHIDCtap2HealthCheck
            // 
            this.buttonHIDCtap2HealthCheck.Location = new System.Drawing.Point(21, 24);
            this.buttonHIDCtap2HealthCheck.Name = "buttonHIDCtap2HealthCheck";
            this.buttonHIDCtap2HealthCheck.Size = new System.Drawing.Size(229, 25);
            this.buttonHIDCtap2HealthCheck.TabIndex = 4;
            this.buttonHIDCtap2HealthCheck.Text = "CTAP2ヘルスチェック実行";
            this.buttonHIDCtap2HealthCheck.UseVisualStyleBackColor = true;
            this.buttonHIDCtap2HealthCheck.Click += new System.EventHandler(this.buttonHIDCtap2HealthCheck_Click);
            // 
            // buttonHIDU2FHealthCheck
            // 
            this.buttonHIDU2FHealthCheck.Location = new System.Drawing.Point(21, 61);
            this.buttonHIDU2FHealthCheck.Name = "buttonHIDU2FHealthCheck";
            this.buttonHIDU2FHealthCheck.Size = new System.Drawing.Size(229, 25);
            this.buttonHIDU2FHealthCheck.TabIndex = 5;
            this.buttonHIDU2FHealthCheck.Text = "U2Fヘルスチェック実行";
            this.buttonHIDU2FHealthCheck.UseVisualStyleBackColor = true;
            this.buttonHIDU2FHealthCheck.Click += new System.EventHandler(this.buttonHIDU2FHealthCheck_Click);
            // 
            // HealthCheckForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 353);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "HealthCheckForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ヘルスチェック実行";
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonBLECtap2HealthCheck;
        private System.Windows.Forms.Button buttonBLEU2FHealthCheck;
        private System.Windows.Forms.Button buttonBLEPingTest;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button buttonHIDPingTest;
        private System.Windows.Forms.Button buttonHIDCtap2HealthCheck;
        private System.Windows.Forms.Button buttonHIDU2FHealthCheck;
    }
}