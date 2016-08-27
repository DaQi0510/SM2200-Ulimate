using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Statistics : Form
    {
        int StartAddress1 = 15;
        int StartAddress2 = 117;
        int StartAddress3 = 212;
        int StartAddress4 = 319;
        public Statistics()
        {
            InitializeComponent();
            InterfaceDisplay();
        }
        /// <summary>
        /// 初始控件显示
        /// </summary>
        void InterfaceDisplay()
        {
            //显示通道
            for (byte i = 0; i < 18; i++)
            {
                Label Lab = new Label();
                Lab.Name = "通道" + (i + 1).ToString();
                Lab.Text = "通道" + (i + 1).ToString() + "：";
                Lab.AutoSize = true;
                Lab.Font = new System.Drawing.Font("宋体", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
                Lab.Location = new System.Drawing.Point(StartAddress1, 40 + 25 * i);
                Lab.Size = new System.Drawing.Size(45, 12);
                groupBox1.Controls.Add(Lab);
            }
            //显示接收数据包数
            for (byte i = 0; i < 18; i++)
            {
                Label Labs = new Label();
                Labs.Name = "StatisPacket" + (i + 1).ToString();
                Labs.Text = G.StatisPacket[i].ToString();
                Labs.AutoSize = true;
                Labs.Font = new System.Drawing.Font("宋体", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
                Labs.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
                Labs.Location = new System.Drawing.Point(StartAddress2, 40 + 25 * i);
                Labs.Size = new System.Drawing.Size(45, 12);
                groupBox1.Controls.Add(Labs);
            }
            //显示收包率，0.95-1绿色  0.9-0.95淡红色 0-0.9红色
            for (byte i = 0; i < 18; i++)
            {
                Label Labr = new Label();
                Labr.Name = "PacketRate" + (i + 1).ToString();
                Labr.Text = Math.Round(G.PacketRate[i], 3).ToString()+"%";
                if (G.PacketRate[i] >= 95)
                {
                    Labr.ForeColor = System.Drawing.Color.Green;
                }
                if ((G.PacketRate[i] >= 90) && (G.PacketRate[i] < 95))
                {
                    Labr.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(128)))));
                }
                if (G.PacketRate[i] < 90)
                {
                    Labr.ForeColor = System.Drawing.Color.Red;
                }
                Labr.AutoSize = true;
                Labr.Font = new System.Drawing.Font("宋体", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
                Labr.Location = new System.Drawing.Point(StartAddress3, 40 + 25 * i);
                Labr.Size = new System.Drawing.Size(45, 12);
                groupBox1.Controls.Add(Labr);
            }
            //通道频点显示
            for (byte i = 0; i < 18; i++)
            {
                Label Labf = new Label();
                Labf.Name = "Frequence" + (i + 1).ToString();
                Labf.Text = G.Frequences[G.Frequence[i] - 3].ToString() + "kHz";
                Labf.AutoSize = true;
                Labf.Font = new System.Drawing.Font("宋体", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
                Labf.Location = new System.Drawing.Point(StartAddress4, 40 + 25 * i);
                Labf.Size = new System.Drawing.Size(45, 12);
                groupBox1.Controls.Add(Labf);
            }
            //设备号显示
            label5.Text = G.Device.ToString();
            //接收总数据包数显示
            label7.Text = G.TolPacket.ToString();
        }
    }
}
