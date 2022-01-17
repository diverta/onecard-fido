namespace MaintenanceToolGUI
{
    partial class CommonProcessingForm
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
            this.LabelProgress = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // LabelProgress
            // 
            this.LabelProgress.Location = new System.Drawing.Point(12, 17);
            this.LabelProgress.Name = "LabelProgress";
            this.LabelProgress.Size = new System.Drawing.Size(285, 26);
            this.LabelProgress.TabIndex = 3;
            this.LabelProgress.Text = "しばらくお待ちください...";
            this.LabelProgress.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LabelProgress.UseWaitCursor = true;
            // 
            // CommonProcessingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(309, 61);
            this.ControlBox = false;
            this.Controls.Add(this.LabelProgress);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CommonProcessingForm";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "処理を実行中です。";
            this.UseWaitCursor = true;
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Label LabelProgress;
    }
}