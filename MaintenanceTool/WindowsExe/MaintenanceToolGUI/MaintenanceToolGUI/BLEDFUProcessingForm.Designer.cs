namespace MaintenanceToolGUI
{
    partial class BLEDFUProcessingForm
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
            this.LevelIndicator = new System.Windows.Forms.ProgressBar();
            this.LabelProgress = new System.Windows.Forms.Label();
            this.ButtonCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // LevelIndicator
            // 
            this.LevelIndicator.Location = new System.Drawing.Point(12, 18);
            this.LevelIndicator.Maximum = 23;
            this.LevelIndicator.Name = "LevelIndicator";
            this.LevelIndicator.Size = new System.Drawing.Size(285, 25);
            this.LevelIndicator.Step = 1;
            this.LevelIndicator.TabIndex = 2;
            this.LevelIndicator.UseWaitCursor = true;
            // 
            // LabelProgress
            // 
            this.LabelProgress.Location = new System.Drawing.Point(12, 55);
            this.LabelProgress.Name = "LabelProgress";
            this.LabelProgress.Size = new System.Drawing.Size(285, 26);
            this.LabelProgress.TabIndex = 3;
            this.LabelProgress.Text = "LabelProgress";
            this.LabelProgress.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LabelProgress.UseWaitCursor = true;
            // 
            // ButtonCancel
            // 
            this.ButtonCancel.Location = new System.Drawing.Point(114, 91);
            this.ButtonCancel.Name = "ButtonCancel";
            this.ButtonCancel.Size = new System.Drawing.Size(80, 23);
            this.ButtonCancel.TabIndex = 1;
            this.ButtonCancel.Text = "Cancel";
            this.ButtonCancel.UseVisualStyleBackColor = true;
            this.ButtonCancel.UseWaitCursor = true;
            this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
            // 
            // BLEDFUProcessingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(309, 126);
            this.ControlBox = false;
            this.Controls.Add(this.ButtonCancel);
            this.Controls.Add(this.LabelProgress);
            this.Controls.Add(this.LevelIndicator);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BLEDFUProcessingForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "ファームウェアを更新しています";
            this.UseWaitCursor = true;
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ProgressBar LevelIndicator;
        private System.Windows.Forms.Label LabelProgress;
        private System.Windows.Forms.Button ButtonCancel;
    }
}