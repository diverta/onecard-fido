﻿namespace MaintenanceToolGUI
{
    partial class DFUProcessingForm
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
            this.SuspendLayout();
            // 
            // LevelIndicator
            // 
            this.LevelIndicator.Location = new System.Drawing.Point(12, 26);
            this.LevelIndicator.Maximum = 20;
            this.LevelIndicator.Name = "LevelIndicator";
            this.LevelIndicator.Size = new System.Drawing.Size(285, 25);
            this.LevelIndicator.Step = 1;
            this.LevelIndicator.TabIndex = 0;
            // 
            // LabelProgress
            // 
            this.LabelProgress.Location = new System.Drawing.Point(12, 68);
            this.LabelProgress.Name = "LabelProgress";
            this.LabelProgress.Size = new System.Drawing.Size(285, 26);
            this.LabelProgress.TabIndex = 12;
            this.LabelProgress.Text = "LabelProgress";
            this.LabelProgress.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // DFUProcessingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(309, 126);
            this.Controls.Add(this.LabelProgress);
            this.Controls.Add(this.LevelIndicator);
            this.Name = "DFUProcessingForm";
            this.Text = "ファームウェアを更新しています";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ProgressBar LevelIndicator;
        private System.Windows.Forms.Label LabelProgress;
    }
}