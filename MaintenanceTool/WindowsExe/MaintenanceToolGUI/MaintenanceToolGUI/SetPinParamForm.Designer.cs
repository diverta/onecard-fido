namespace MaintenanceToolGUI
{
    partial class SetPinParamForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.textPin = new System.Windows.Forms.TextBox();
            this.textPinConfirm = new System.Windows.Forms.TextBox();
            this.textPinOld = new System.Windows.Forms.TextBox();
            this.buttonChangePin = new System.Windows.Forms.Button();
            this.buttonSetPin = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(81, 12);
            this.label1.TabIndex = 8;
            this.label1.Text = "新しいPINコード";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 51);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(113, 12);
            this.label2.TabIndex = 9;
            this.label2.Text = "新しいPINコード(確認)";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 80);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(96, 12);
            this.label3.TabIndex = 10;
            this.label3.Text = "変更前のPINコード";
            // 
            // textPin
            // 
            this.textPin.Location = new System.Drawing.Point(144, 19);
            this.textPin.Name = "textPin";
            this.textPin.Size = new System.Drawing.Size(228, 19);
            this.textPin.TabIndex = 1;
            this.textPin.UseSystemPasswordChar = true;
            // 
            // textPinConfirm
            // 
            this.textPinConfirm.Location = new System.Drawing.Point(144, 48);
            this.textPinConfirm.Name = "textPinConfirm";
            this.textPinConfirm.Size = new System.Drawing.Size(228, 19);
            this.textPinConfirm.TabIndex = 2;
            this.textPinConfirm.UseSystemPasswordChar = true;
            // 
            // textPinOld
            // 
            this.textPinOld.Location = new System.Drawing.Point(144, 77);
            this.textPinOld.Name = "textPinOld";
            this.textPinOld.Size = new System.Drawing.Size(228, 19);
            this.textPinOld.TabIndex = 3;
            this.textPinOld.UseSystemPasswordChar = true;
            // 
            // buttonChangePin
            // 
            this.buttonChangePin.Location = new System.Drawing.Point(154, 121);
            this.buttonChangePin.Name = "buttonChangePin";
            this.buttonChangePin.Size = new System.Drawing.Size(75, 23);
            this.buttonChangePin.TabIndex = 5;
            this.buttonChangePin.Text = "変更";
            this.buttonChangePin.UseVisualStyleBackColor = true;
            this.buttonChangePin.Click += new System.EventHandler(this.buttonChangePin_Click);
            // 
            // buttonSetPin
            // 
            this.buttonSetPin.Location = new System.Drawing.Point(62, 121);
            this.buttonSetPin.Name = "buttonSetPin";
            this.buttonSetPin.Size = new System.Drawing.Size(75, 23);
            this.buttonSetPin.TabIndex = 4;
            this.buttonSetPin.Text = "新規設定";
            this.buttonSetPin.UseVisualStyleBackColor = true;
            this.buttonSetPin.Click += new System.EventHandler(this.buttonSetPin_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(246, 121);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 7;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // SetPinParamForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 156);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonSetPin);
            this.Controls.Add(this.buttonChangePin);
            this.Controls.Add(this.textPinOld);
            this.Controls.Add(this.textPinConfirm);
            this.Controls.Add(this.textPin);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SetPinParamForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "PINコード設定";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textPin;
        private System.Windows.Forms.TextBox textPinConfirm;
        private System.Windows.Forms.TextBox textPinOld;
        private System.Windows.Forms.Button buttonChangePin;
        private System.Windows.Forms.Button buttonSetPin;
        private System.Windows.Forms.Button buttonCancel;
    }
}