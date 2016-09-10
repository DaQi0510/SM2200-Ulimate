using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace WindowsFormsApplication1
{
    public partial class Statistics : Form
    {
        int StartAddress1 = 10;
        int StartAddress2 = 160;
        int StartAddress3 = 242;
        int StartAddress4 = 332;
        public Statistics()
        {
            InitializeComponent();
            InterfaceDisplay();
            label11.Text = System.DateTime.Now.ToString("yyyy年MM月dd日HH时mm分ss秒"); 
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
                Lab.Text = "通道" + (i + 1).ToString() + "("+G.Frequences[G.Frequence[i] - 3].ToString() + "kHz"+"）：";
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
            //信道噪声显示
            for (byte i = 0; i < 18; i++)
            {
                Label Labf = new Label();
                Labf.Name = "Rate" + (i + 1).ToString();
                Labf.Text = G.Noise[i].ToString ();
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
            label8.Text = Math.Round(G.Rates, 3).ToString() + " kbps";
        }
        //保存成txt文档
        private void button1_Click(object sender, EventArgs e)
        {
            string filename;
            SaveFileDialog SaveFile = new SaveFileDialog();
            SaveFile.Filter="txt files (*.txt))|*.txt";       //打开需要读取的文件
            StringBuilder Information = new StringBuilder();
            StringBuilder Line = new StringBuilder();
            int Length1=3;//信道前面长度
            int Length2=24;//接收包前面长度
            int Length3=47;//收包率前面长度
            int Length4=70;//信道噪声前面长度
            SaveFile.FileName = label11 .Text+"(设备"+G.Device.ToString() +")";
            if (SaveFile.ShowDialog() == DialogResult.OK)  //选择的是确定按钮
            {
                filename = SaveFile.FileName;
                Information.AppendFormat("调取时间：{0}。设备号：{1}。", label11.Text, G.Device.ToString());
                Information.Append(System.Environment.NewLine);
                Information.AppendFormat("数据包总数：{0}" + System.Environment.NewLine, G.TolPacket.ToString());
                Information.Append("   信道：                  接收包：               收包率：               信道噪声：" + System.Environment.NewLine);
                for (byte i = 0; i < 18; i++)
                {
                    Line.Length = 0;
                    while (Line.Length < Length1)
                    {
                        Line.Append(" ");
                    }
                    Line.Append("通道" + (i + 1).ToString() + "("+G.Frequences[G.Frequence[i] - 3].ToString() + "kHz）");
                    while (Line.Length < Length2)
                    {
                        Line.Append(" ");
                    }
                    Line.Append(G.StatisPacket[i].ToString());
                    while (Line.Length < Length3)
                    {
                        Line.Append(" ");
                    }
                    Line.Append(Math.Round(G.PacketRate[i], 3).ToString() + "%");
                    while (Line.Length < Length4)
                    {
                        Line.Append(" ");
                    }
                    Line.Append(G.Noise[i].ToString());
                    Information.Append(Line + System.Environment.NewLine);
                    //Line.Append("通道" + (i + 1).ToString() + "(" + G.Frequences[G.Frequence[i] - 3].ToString() + "kHz）");
                    //Information.AppendFormat("         {0}","通道" + (i + 1).ToString() + "("+G.Frequences[G.Frequence[i] - 3].ToString() + "kHz）");
                    //Information.AppendFormat("                             {0}", G.StatisPacket[i].ToString());
                    //Information.AppendFormat("                                                     {0}", Math.Round(G.PacketRate[i], 3).ToString() + "%");
                    //Information.AppendFormat("                                                                            {0}" + System.Environment.NewLine, G.Noise[i].ToString());
                }
                FileStream Fs = new FileStream(filename, FileMode.Create, FileAccess.Write);//创建写入文件 
                StreamWriter Sr = new StreamWriter(Fs);
                Sr.WriteLine(Information.ToString());//开始写入值
                Sr.Close();
                Fs.Close();
            }
        }
    }
}
