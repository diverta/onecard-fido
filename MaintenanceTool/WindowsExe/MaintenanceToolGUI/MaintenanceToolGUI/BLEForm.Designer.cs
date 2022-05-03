namespace MaintenanceToolGUI
{
    partial class BLEForm
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
            this.buttonPairing = new System.Windows.Forms.Button();
            this.buttonUnpairing = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(108, 173);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(79, 23);
            this.buttonCancel.TabIndex = 3;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.buttonPairing);
            this.groupBox1.Controls.Add(this.buttonUnpairing);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(268, 141);
            this.groupBox1.TabIndex = 19;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "BLEペアリング";
            // 
            // buttonPairing
            // 
            this.buttonPairing.Location = new System.Drawing.Point(21, 32);
            this.buttonPairing.Name = "buttonPairing";
            this.buttonPairing.Size = new System.Drawing.Size(229, 25);
            this.buttonPairing.TabIndex = 1;
            this.buttonPairing.Text = "ペアリング実行";
            this.buttonPairing.UseVisualStyleBackColor = true;
            this.buttonPairing.Click += new System.EventHandler(this.buttonPairing_Click);
            // 
            // buttonUnpairing
            // 
            this.buttonUnpairing.Location = new System.Drawing.Point(21, 88);
            this.buttonUnpairing.Name = "buttonUnpairing";
            this.buttonUnpairing.Size = new System.Drawing.Size(229, 25);
            this.buttonUnpairing.TabIndex = 2;
            this.buttonUnpairing.Text = "ペアリング解除";
            this.buttonUnpairing.UseVisualStyleBackColor = true;
            this.buttonUnpairing.Click += new System.EventHandler(this.buttonUnpairing_Click);
            // 
            // BLEForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 218);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BLEForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "BLE設定";
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonPairing;
        private System.Windows.Forms.Button buttonUnpairing;
    }
}