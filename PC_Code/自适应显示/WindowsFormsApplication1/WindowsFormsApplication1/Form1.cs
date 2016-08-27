using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        #region        命令定义部分 
        static byte Connect = 0x00;         //命令地址0x00，建立连接
        static byte DisConnect = 0x01;      //命令地址0x01，断开连接
        static byte PollChat = 0x02;        //轮询通信方式
        static byte RunModes = 0x03;       //命令地址0x03, 运行模式配置  
        //static byte Message_Send = 0x02;    //载波设备发送信息
        //static byte Message_Rece = 0x03;    //载波设备接收信息
        //static byte CheckChannel = 0x04;    //查找可用通道模式
        //static byte SendRece = 0x05;        //收发数据模式
        #endregion
        IPAddress IP_Device;
        IPEndPoint EndPoint_Device = null;
        static Socket Socket_Device = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        static byte[] RData = new byte[128];
        static byte[] TData = new byte[128];  
        string IP1 = "192.168.1.61";
        int Port1 = 3590+20*1;
        string IP2 = "192.168.1.63";
        int Port2 = 3590 + 20 * 2;
        string IP3 = "192.168.1.65";
        int Port3 = 3590 + 20 * 3;
        string IP4 = "192.168.1.67";
        int Port4 = 3590 + 20 * 4;
        string IP5 = "192.168.1.69";
        int Port5 = 3590 + 20 * 5;
        string IP6 = "192.168.1.71";
        int Port6 = 3590 + 20 * 6;
        string IP7 = "192.168.1.73";
        int Port7 = 3590 + 20 * 7;
        string IP8 = "192.168.1.75";
        int Port8 = 3590 + 20 * 8;
        static StringBuilder Information =new StringBuilder (); //临时存储文本框每行需要显示的信息
        static byte  ConnectDevice;         //连接的设备号
        static byte[]  ConnectToDevices =new byte[7];       //连接的设备号
        static byte  StartVoltage;          //起始电压
        static byte DeviceScale;            //从属级别
        static byte SendScale;              //发送主从级别
        static byte SendVoltage;            //发送电压
        static int DataLength;              //接收到数据长度
        static int num;
        static int numlast;
        static bool ConnectFlag;
        static bool CommandFlag;
        static int Numi;
        static byte Numl;
        static byte RunMode;
        public Form1()
        {
            InitializeComponent();
            timer1.Enabled = true;
            timer1.Start();
            Receive .Event +=new EventHandler(Receive_Event);
            Info .Event +=new EventHandler(Info_Event);
            ConnectFlag = false;
            //建立通信设备号
            ConnectDevice = 1;
  
            //主从关系
            DeviceScale = 1;
            comboBox2.Text = "主";
            comboBox2.Items.Add("主");
            comboBox2.Items.Add("从");
            comboBox2.SelectedIndex = 0;
            //发送主从级别
            SendScale  = 1;
            //初始幅值
            textBox1.Text = "3";
            StartVoltage = 3;
            SendVoltage = 3;
            Control.CheckForIllegalCrossThreadCalls = false;
        }
        #region  选择需要连接的设备号
        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            if(radioButton1.Checked ==true)
            {
                IP_Device = IPAddress.Parse(IP1);
                EndPoint_Device = new IPEndPoint(IP_Device, Port1);
                ConnectDevice = 1;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton2.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP2);
                EndPoint_Device = new IPEndPoint(IP_Device, Port2);
                ConnectDevice = 2;
                ChatDeviceDisplay();
                StatisticsDisplay(); 
            }
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton3.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP3);
                EndPoint_Device = new IPEndPoint(IP_Device, Port3);
                ConnectDevice = 3;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }

        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton4.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP4);
                EndPoint_Device = new IPEndPoint(IP_Device, Port4);
                ConnectDevice = 4;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }
        private void radioButton5_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton5.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP5);
                EndPoint_Device = new IPEndPoint(IP_Device, Port5);
                ConnectDevice = 5;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }
        private void radioButton6_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton6.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP6);
                EndPoint_Device = new IPEndPoint(IP_Device, Port6);
                ConnectDevice = 6;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }
        private void radioButton7_CheckedChanged(object sender, EventArgs e)
        {
             if (radioButton7.Checked == true)
             {
                 IP_Device = IPAddress.Parse(IP7);
                 EndPoint_Device = new IPEndPoint(IP_Device, Port7);
                 ConnectDevice = 7;
                 ChatDeviceDisplay();
                 StatisticsDisplay();
             }
        }
        private void radioButton8_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton8.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP8);
                EndPoint_Device = new IPEndPoint(IP_Device, Port8);
                ConnectDevice = 8;
                ChatDeviceDisplay();
                StatisticsDisplay();
            }
        }
        #endregion

        #region  设备运行方式
        /// <summary>
        /// 运行模式选择-轮询方式
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RadButton1_CheckedChanged(object sender, EventArgs e)
        {
            if (RadButton1.Checked == true)
                RunMode = 1;
        }
        /// <summary>
        /// 运行模式选择-联网运行
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void RadButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (RadButton2.Checked == true)
                RunMode = 2;
        }
        /// <summary>
        /// 运行模式配置
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button4_Click_1(object sender, EventArgs e)
        {
            TData[0] = 0x3C;  //<
            TData[1] = RunModes ;
            TData[2] = RunMode ;
            TData[7] =0x3E;
            Information.Length = 0;
            Information.Append(System.DateTime.Now.ToString() + " ");
            Information.Append("运行模式配置！");
            Information.Append(System.Environment.NewLine);
            Info.Update = true;
            SendMessage(8);
        }
        #endregion

        private void button1_Click(object sender, EventArgs e)     //与指定设备进行连接
        {
            Information.Append(System .DateTime .Now .ToString ()+" ");
            Information.Append("尝试与设备 " + ConnectDevice.ToString() + " 建立网络连接" + System.Environment.NewLine);
            Info.Update = true;
            try
            {
                Socket_Device.BeginConnect(EndPoint_Device, new AsyncCallback(ConnectCallback_Device), Socket_Device);
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" +"建立网络连接失败"  +System.Environment.NewLine);
                Info.Update = true;
            }
        }

        #region        通信设置部分
        /// <summary>
        /// 设备从属级别设置
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e) //设备从属级别设置
        {

            if (comboBox2.SelectedIndex == 0)
            {
                DeviceScale = 1;
                ChatDeviceEnable(true);
                ChatDeviceDisplay();
            }
            if (comboBox2.SelectedIndex == 1)
            {
                DeviceScale = 0;
                ChatDeviceEnable(false);
                ChatDeviceDisplay();
            }
            StatisticsEnable();
        }
        /// <summary>
        /// 控制通信设备按钮状态
        /// </summary>
        /// <param name="flag" flag=true使能 flag=flase失能></param>
        void ChatDeviceEnable(bool flag)   //控制通信设备按钮状态
        {
            if (flag == true)
            {
                btn1.Enabled = true;
                btn2.Enabled = true;
                btn3.Enabled = true;
                btn4.Enabled = true;
                btn5.Enabled = true;
                btn6.Enabled = true;
                btn7.Enabled = true;
            }
            else
            {
                btn1.Enabled = false ;
                btn2.Enabled = false;
                btn3.Enabled = false;
                btn4.Enabled = false;
                btn5.Enabled = false;
                btn6.Enabled = false;
                btn7.Enabled = false;
                btn1.BackColor = System.Drawing.SystemColors.ControlLight;
                btn2.BackColor = System.Drawing.SystemColors.ControlLight;
                btn3.BackColor = System.Drawing.SystemColors.ControlLight;
                btn4.BackColor = System.Drawing.SystemColors.ControlLight;
                btn5.BackColor = System.Drawing.SystemColors.ControlLight;
                btn6.BackColor = System.Drawing.SystemColors.ControlLight;
                btn7.BackColor = System.Drawing.SystemColors.ControlLight;
                for (int i = 1; i < 8; i++)
                {
                    ConnectToDevices[i - 1] = 0;
                }
            }
        }
        /// <summary>
        /// 载波通信设备按钮显示
        /// </summary>
        void ChatDeviceDisplay()
        {
            if (DeviceScale == 0)
            {
                btn1.Text = "*";
                btn2.Text = "*";
                btn3.Text = "*";
                btn4.Text = "*";
                btn5.Text = "*";
                btn6.Text = "*";
                btn7.Text = "*";
            }
            else
            {
                for(byte i=1;i<8;i++)
                {
                    foreach (Control ctl in this.groupBox2.Controls)   //遍历通信设置部分所有控件
                    {
                        if (ctl is Button)
                        {
                            if (ctl.Name == ("btn" + i.ToString()))
                            {
                                if (i < ConnectDevice)
                                    ctl.Text = i.ToString();
                                else
                                {
                                    ctl.Text = (i + 1).ToString();
                                }
                            }  
                        }
                    }
                }
            }  
        }
        /// <summary>
        /// 按钮触发事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btn_Click(object sender, EventArgs e)
        {
            Button btn =(Button)sender ;
            for (byte i = 1; i < 8; i++)
            {
                if (btn.Name == "btn" + i.ToString())
                {
                    if (ConnectToDevices[i-1] == 0)
                    {
                        ConnectToDevices[i-1] = byte.Parse(btn.Text);
                        btn.BackColor = System.Drawing.Color.DarkOrange;
                    }
                    else
                    {
                        ConnectToDevices[i-1] = 0;
                        btn.BackColor = System.Drawing.SystemColors.ControlLight;
                    }
                }
            }
            StatisticsEnable();
        }

        /// <summary>
        /// 通信设置
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button3_Click(object sender, EventArgs e) //通信设置
        {
            TData[0] = 0x3C;  //<
            TData[1] =PollChat ;
            TData[2] =0x00;
            TData[3] = DeviceScale;
            for (byte i = 1; i < 8; i++)
            {
                TData[3 + i] = ConnectToDevices[i - 1];
            }
            TData[11] = StartVoltage;
            TData[15] = 0x3E; //>
            Information.Length = 0;
            Information.Append(System.DateTime.Now.ToString() + " ");
            if (DeviceScale == 1)    //主动模式
            {
                Information.Append("主模式，与设备： ");
                for (byte i = 1; i < 8; i++)
                {
                    if (ConnectToDevices[i - 1] != 0)
                        Information.Append(ConnectToDevices[i - 1].ToString()+" ");
                }
                Information.Append("建立载波通信，电压幅值设置： "+StartVoltage .ToString ());
            }
            if (DeviceScale == 0)    //从模式
            {
                Information.Append("从模式，等待主设备与之通信。电压幅值设置： " + StartVoltage.ToString());
            }
            Information.Append(System.Environment.NewLine);
            Info.Update = true;
            SendMessage(16);
        }
        /// <summary>
        /// 开始幅值设定
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void textBox1_TextChanged(object sender, EventArgs e) //开始幅值设定
        {
            StartVoltage = (byte)int.Parse(textBox1.Text.Trim());
        }
        /// <summary>
        /// 读取设备通信配置信息
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button5_Click(object sender, EventArgs e)
        {
            TData[0] = 0x3C;  //<
            TData[1] = PollChat ;
            TData[2] = 0x01;
            TData[15] = 0x3E; //>
            Information.Length = 0;
            Information.Append(System.DateTime.Now.ToString() + " ");
            Information.Append("获取通信配置信息！");
            Information.Append(System.Environment.NewLine);
            Info.Update = true;
            SendMessage(16);
        }
        /// <summary>
        /// 根据设备回传信息显示
        /// </summary>
        void ChatDeviceDisplays()
        {
            if (DeviceScale == 1)
            {
                ChatDeviceEnable(true);
                comboBox2.SelectedIndex = 0;
            }
            else
            {
                ChatDeviceEnable(false);
                comboBox2.SelectedIndex = 1;
                btn1.Text = "*";
                btn2.Text = "*";
                btn3.Text = "*";
                btn4.Text = "*";
                btn5.Text = "*";
                btn6.Text = "*";
                btn7.Text = "*";
            }
            for (byte i = 1; i < 8; i++)
            {
                foreach (Control ctl in this.groupBox2.Controls)   //遍历通信设置部分所有控件
                {
                    if (ctl is Button)
                    {
                        if (ctl.Name == ("btn" + i.ToString()))
                        {
                            if (ConnectToDevices[i - 1] != 0)
                                ctl.BackColor = System.Drawing.Color.DarkOrange;
                            else
                                ctl.BackColor = System.Drawing.SystemColors.ControlLight;

                        }
                    }
                }
            }
            textBox1.Text = StartVoltage.ToString();

        }
        #endregion

        #region        网口程序部分
        /// <summary>
        /// 设备建立连接回调函数
        /// </summary>
        /// <param name="er"></param>
        private static void ConnectCallback_Device(IAsyncResult er)   //设备建立连接回调函数
        {
            try
            {
                Socket Socket_Device1 = (Socket)er.AsyncState;
                Socket_Device1.EndConnect(er);
                DelayMs(10);
                TData[0] = 0x3C;  //<
                TData[1] = Connect;
                TData[7] = 0x3E;  //>
                DelayMs(5);
                SendMessage(8);
                ReceiveMessage();
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + "建立网络连接失败" + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        /// <summary>
        /// 接收设备发送数据
        /// </summary>
        private static void ReceiveMessage()       //接收设备发送数据
        {
            try
            {
                Socket_Device.BeginReceive(RData, 0, 128, 0, new AsyncCallback(ReceiveCallback_Device), Socket_Device);
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + "接收网络数据失败" + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        /// <summary>
        /// 接收数据回调函数
        /// </summary>
        /// <param name="er"></param>
        private static void ReceiveCallback_Device(IAsyncResult er)    //接收数据回调函数
        {
            try
            {
                Socket Socket_Device1 = (Socket)er.AsyncState;
                DataLength = Socket_Device1.EndReceive(er);
                ReceiveMessage();
                Receive.Received = true;
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + "接受网络数据失败" + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        /// <summary>
        /// 发送数据函数
        /// </summary>
        /// <param name="SendNum"></param>
        private static void SendMessage(int SendNum)    //发送数据
        {
            try
            {
                Socket_Device.BeginSend(TData, 0, SendNum, 0, new AsyncCallback(SendCallback_Device), Socket_Device);
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + "发送网络数据失败" + System.Environment.NewLine);
                Info.Update = true;
            }

        }
        /// <summary>
        /// 设备发送数据回调函数
        /// </summary>
        /// <param name="er"></param>
        private static void SendCallback_Device(IAsyncResult er)  //设备发送数据回调函数
        {
            try
            {
                Socket Socket_Device1 = (Socket)er.AsyncState;
                Socket_Device1.EndSend(er);
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + "发送网络数据失败" + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        #endregion

        #region        运行统计部分
        /// <summary>
        /// 统计界面显示
        /// </summary>
        void StatisticsDisplay()
        {
            for(byte i=1;i<=7;i++)
            {
                foreach (Control ctl in this.groupBox4.Controls)   //遍历通信设置部分所有控件
                {
                    if (ctl is Button)
                    {
                        if (ctl.Name == ("Btn1" + i.ToString()))
                        {
                            if (i < ConnectDevice)
                                ctl.Text ="设备" + i.ToString();
                            else
                            {
                                ctl.Text = "设备" + (i + 1).ToString();
                            }
                        }  
                    }
                }
            }
        }
        /// <summary>
        /// 统计界面按钮使能
        /// </summary>
        void StatisticsEnable()
        {
            for (byte i = 0; i < 7; i++)
            {
                foreach (Control ctl in this.groupBox4.Controls)   //遍历通信设置部分所有控件
                {
                    if (ctl is Button)
                    {
                        if (ctl.Name == ("Btn1" + (i+1).ToString()))
                        {
                            if (ConnectToDevices[i] != 0)
                                ctl.Enabled = true;
                            else
                                ctl.Enabled = false ;
                        }
                    }
                }
            }
            if (DeviceScale == 0)
                Btn18.Enabled = true;
            else
                Btn18.Enabled = false;
        }
        /// <summary>
        /// 按键功能部分
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Btn1_Click(object sender, EventArgs e)
        {
            //找到触发事件的按键
            Button btn = (Button)sender;
            for (byte i = 1; i < 8; i++)
            {
                if (btn.Name == "Btn1" + i.ToString())
                {
                    TData[0] = 0x3C;  //<
                    TData[1] = PollChat;
                    TData[2] = 0x02;
                    TData[3] = ConnectToDevices[i-1]; //>
                    TData[7] = 0x3E; //>
                    Information.Length = 0;
                    Information.Append(System.DateTime.Now.ToString() + " ");
                    Information.Append("获取设备 " + ConnectToDevices[i-1].ToString ()+" 数据包统计信息！");
                    Information.Append(System.Environment.NewLine);
                    Info.Update = true;
                    SendMessage(8);
                }
            }
            //从模式下调取本设备数据包统计
            if (btn.Name == "Btn18")
            {
                TData[0] = 0x3C;  //<
                TData[1] = PollChat;
                TData[2] = 0x02;
                TData[3] = ConnectDevice; //>
                TData[7] = 0x3E; //>
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("获取本设备 " + ConnectDevice.ToString() + " 数据包统计信息！");
                Information.Append(System.Environment.NewLine);
                Info.Update = true;
                SendMessage(8);
            }
        }
        #endregion

        void Info_Event(object sender, EventArgs e) //接收到设备数据触发事件
        {
            richTextBox1.AppendText(Information.ToString());       //显示错误详细信息
        }

        void Receive_Event(object sender, EventArgs e)    //接收到设备数据触发事件
        {
            byte ChannelNum;
            int Kong;
            if((RData[0]==0x3C)&&(RData[DataLength-1]==0x3E)) //判断数据包是否满足约定首尾格式
            {
                switch (RData[1])
                {
                    case 0x00  :  //建立连接
                        Information.Length = 0;
                        Information.Append(System .DateTime .Now .ToString ()+" ");
                        Information.Append("与设备 " + ConnectDevice.ToString() + " 成功建立网络连接" + System.Environment.NewLine);
                        Info.Update = true;  
                        break;
                    case 0x01 : //断开连接
                        Socket_Device.Dispose();
                        Information.Length = 0;
                        Information.Append(System .DateTime .Now .ToString ()+" ");
                        Information.Append("与设备 " + ConnectDevice.ToString() + " 断开网络连接" + System.Environment.NewLine);
                        Info.Update = true;
                        break;
                    case 0x02:      //轮询通信部分
                        if (RData[2] == 0x00)     //通信配置部分
                        {
                            Information.Length = 0;
                            Information.Append(System.DateTime.Now.ToString() + " ");
                            Information.Append("通信配置成功！ ");
                            if (DeviceScale == 1)
                            {
                                Information.Append("主模式 通信设备 ");
                                for (byte i = 0; i < 7; i++)
                                {
                                    if (RData[i + 4] != 0)
                                        Information.Append(RData[i + 4].ToString() + " ");
                                }
                                Information.Append("信号强度： " + RData[11].ToString());
                            }
                            else
                            {
                                Information.Append("从模式,");
                                Information.Append("信号强度： " + RData[11].ToString());
                            }
                            Information.Append(System.Environment.NewLine);
                            Info.Update = true;
                        }
                        if (RData[2] == 0x01)     //获取通信配置部分
                        {
                            Information.Length = 0;
                            Information.Append(System.DateTime.Now.ToString() + " ");
                            Information.Append("获取通信配置信息成功！");
                            Information.Append(System.Environment.NewLine);
                            Info.Update = true;
                            DeviceScale = RData[3];
                            for (byte i = 0; i < 7; i++)
                            {
                                ConnectToDevices[i] = RData[4 + i];
                            }
                            StartVoltage = RData[11];
                            ChatDeviceDisplays();
                            StatisticsEnable();
                        }
                        if (RData[2] == 0x02)     //获取数据包统计信息
                        {
                            Information.Length = 0;
                            Information.Append(System.DateTime.Now.ToString() + " ");
                            Information.Append("获取数据包统计信息成功！");
                            Information.Append(System.Environment.NewLine);
                            Info.Update = true;
                            G.Device = RData[3];
                            G.TolPacket = 0;
                            //数据包总数
                            for(byte i=0;i<4;i++)
                            {
                                G.TolPacket = G.TolPacket << 8;
                                G.TolPacket += RData[4 + i];
                            }
                            //各通道接收数据包数
                            for (byte i = 0; i < 18; i++)
                            {
                                G.StatisPacket[i] = 0;
                                for (byte j = 0; j < 4; j++)
                                {
                                    G.StatisPacket[i] = G.StatisPacket[i] << 8;
                                    G.StatisPacket[i] += RData[8+4*i+j];
                                }
                                G.Frequence [i]= RData[80+i];   //通信频点统计
                                G.PacketRate[i] = 100*((float)G.StatisPacket[i]) / ((float)G.TolPacket);
                            }
                        }
                        break;
                    case 0x03:      //设备运行模式设置
                        Information.Length = 0;
                        Information.Append(System.DateTime.Now.ToString() + " ");
                        Information.Append("设备运行模式设置成功！ ");
                        Information.Append(System.Environment.NewLine);
                        Info.Update = true;
                        break;
                //        Information.Length = 0;
                //        Information.Append(System.DateTime.Now.ToString() + " ");
                //        Information.Append("发送数据到设备 " + (RData[2] * 256 + RData[3]).ToString() + " 幅值：" + RData[40].ToString() + " " + System.Environment.NewLine);
                //        Information.Append("通道：");
                //        Kong = 0;
                //        for (byte i = 0; i < 18; i++)
                //        {
                //            if (RData[2 * i + 4] != 0)
                //            {
                //                ChannelNum = RData[2 * i + 4];
                //                while (Information.Length - Kong <= 3)
                //                    Information.Append(" ");
                //                Kong = Information.Length;
                //                Information.Append(ChannelNum.ToString());

                //            }
                //        }
                //        Information.Append(System.Environment.NewLine);
                //        Information.Append("频点：");
                //        Kong = 0;
                //        for (byte i = 0; i < 18; i++)
                //        {
                //            if (RData[2 * i + 5] != 0)
                //            {
                //                ChannelNum = RData[2 * i + 5];
                //                while (Information.Length + -Kong <= 3)
                //                    Information.Append(" ");
                //                Kong = Information.Length;
                //                Information.Append(ChannelNum.ToString());
                //            }
                //        }
                //        Information.Append(System.Environment.NewLine);
                //        Info.Update = true;
                //        TData[0] = 0x3C;  //<
                //        //TData[1] = Message_Send;
                //        TData[2] = 0x3E; //>
                //        SendMessage(3);     //回传确认信息
                //        break;
                //    case 0x03:      //设备接收信息
                //        Information.Length = 0;
                //        Information.Append(System.DateTime.Now.ToString() + " ");
                //        Information.Append("接收设备 " + (RData[2] * 256 + RData[3]).ToString() + "数据，" + " 幅值：" + RData[40].ToString() + " " + System.Environment.NewLine);
                //        Information.Append("通道：");
                //        Kong = 0;
                //        for (byte i = 0; i < 18; i++)
                //        {
                //            if (RData[2 * i + 4] != 0)
                //            {
                //                ChannelNum = RData[2 * i + 4];
                //                while (Information.Length - Kong <= 3)
                //                    Information.Append(" ");
                //                Kong = Information.Length;
                //                Information.Append(ChannelNum.ToString() + " ");
                //            }
                //        }
                //        Information.Append(System.Environment.NewLine);
                //        Information.Append("频点：");
                //        Kong = 0;
                //        for (byte i = 0; i < 18; i++)
                //        {
                //            if (RData[2 * i + 5] != 0)
                //            {
                //                ChannelNum = RData[2 * i + 5];
                //                while (Information.Length - Kong <= 3)
                //                    Information.Append(" ");
                //                Kong = Information.Length;
                //                Information.Append(ChannelNum.ToString() + " ");
                //            }
                //        }
                //        Information.Append(System.Environment.NewLine);
                //        Info.Update = true;
                //        TData[0] = 0x3C;  //<
                //        //  TData[1] = Message_Rece ;
                //        TData[2] = 0x3E; //>
                //        SendMessage(3);     //回传确认信息
                //        break;
                //    case 0x04:             //通信频点信息查找
                //        Information.Length = 0;
                //        Information.Append(System.DateTime.Now.ToString() + " ");
                //        Information.Append("设备配置成功！");
                //        Information.Append(System.Environment.NewLine);
                //        Info.Update = true;
                //        break;
                }
            }
        }
    
       
        //断开连接
        private void button2_Click(object sender, EventArgs e)
        {
            TData[0] = 0x3C;  //<
            TData[1] = DisConnect;
            TData[7] = 0x3E; //>
            SendMessage(8);
        }

       
        public static void DelayMs(int MillSecond)   //延时函数，延时时间Ms
        {
            int Start = Environment.TickCount;
            while (System.Math.Abs(Environment.TickCount - Start) < MillSecond)
            {
                Application.DoEvents();
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            //TData[0] = 0x3C;  //<
            //TData[1] = SendRece ;  //<
            //TData[2] = SendVoltage ;
            //TData[3] = 0x3E; //>
            //Information.Length = 0;
            //Information.Append(System.DateTime.Now.ToString() + " ");
            //Information.Append("开始收发模式：");
            //if (SendScale  == 1)
            //{
            //    Information.Append("主");
            //}
            //else
            //{
            //    Information.Append("从");
            //}
            //Information.Append(System.Environment.NewLine);
            //Info.Update = true;
            //SendMessage(4);
        }
        private void timer1_Tick(object sender, EventArgs e)
        {
            if (ConnectFlag == true)
            {
                Numl++;
                if (Numl > 155)
                    Numl = 0;
                for (byte i = 0; i < 100; i++)
                {
                   
                    TData[i] =Convert.ToByte(Numl + i);
                }
                SendMessage(100);
            //   label4.Text = Numl.ToString();
            }
          //  label6.Text = Numi.ToString();
        }


        private void button2_Click_2(object sender, EventArgs e)
        {
            Form Statis = new Statistics();
            Statis.Show();
        }

       


      

       
    }
    class Info         //为更新显示信息添加驱动事件
    {
        public static event EventHandler Event;
        public static bool update;
        public static bool Update
        {
            get
            {
                return update;
            }
            set
            {
                update = value;
                if (update == true)
                    OnUpdate(new EventArgs());
            }
        }
        private static void OnUpdate(EventArgs eventArgs)
        {
            if (Event != null)//判断事件是否有处理函数 
            {
                Event(Update, eventArgs);
            }
        }
    }
    class Receive      //为接收到设备信息添加驱动事件
    {
        /**************回传设备*************/
        public static event EventHandler Event;
        public static bool messagereceive;
        public static bool Received
        {
            get
            {
                return messagereceive;
            }
            set
            {
                messagereceive = value;
                if (messagereceive == true)
                    OnMessagereceive(new EventArgs());
            }
        }
        private static void OnMessagereceive(EventArgs eventArgs)
        {
            if (Event != null)//判断事件是否有处理函数 
            {
                Event(Received, eventArgs);
            }
        }
    }
}
