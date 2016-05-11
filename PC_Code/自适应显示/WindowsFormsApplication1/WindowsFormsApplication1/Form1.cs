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
        static byte Message_Send = 0x02;    //载波设备发送信息
        static byte Message_Rece = 0x03;    //载波设备接收信息
        #endregion
        IPAddress IP_Device;
        IPEndPoint EndPoint_Device = null;
        static Socket Socket_Device = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        static byte[] RData = new byte[128];
        static byte[] TData = new byte[128];  
        string IP1 = "192.168.1.61";
        int Port1 = 3610;
        string IP2 = "192.168.1.63";
        int Port2 = 3630;
        string IP3 = "192.168.1.65";
        int Port3 = 3650;
        string IP4 = "192.168.1.67";
        int Port4 = 3670;
        static StringBuilder Information =new StringBuilder (); //临时存储文本框每行需要显示的信息
        static byte  ConnectDevice;         //连接的设备号
        static int DataLength;              //接收到数据长度
        static int num;
        static int numlast;
        static bool ConnectFlag;
        static bool CommandFlag;
      
        public Form1()
        {
            InitializeComponent();
            Receive .Event +=new EventHandler(Receive_Event);
            Info .Event +=new EventHandler(Info_Event);
            ConnectDevice = 1;

            comboBox1.Text = "1";
            comboBox1.Items.Add(1);
            comboBox1.Items.Add(2);
            comboBox1.Items.Add(3);
            comboBox1.Items.Add(4);
            comboBox1.SelectedIndex = 0;
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
            }
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton2.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP2);
                EndPoint_Device = new IPEndPoint(IP_Device, Port2);
                ConnectDevice = 2;
            }
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton3.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP3);
                EndPoint_Device = new IPEndPoint(IP_Device, Port3);
                ConnectDevice = 3;
            }
        }

        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton4.Checked == true)
            {
                IP_Device = IPAddress.Parse(IP4);
                EndPoint_Device = new IPEndPoint(IP_Device, Port4);
                ConnectDevice = 4;
            }
        }
        #endregion
        private void button1_Click(object sender, EventArgs e)     //与指定设备进行连接
        {
            Information.Append(System .DateTime .Now .ToString ()+" ");
            Information.Append("尝试与设备 " + ConnectDevice.ToString() + " 建立连接" + System.Environment.NewLine);
            Info.Update = true;
            try
            {
                Socket_Device.BeginConnect(EndPoint_Device, new AsyncCallback(ConnectCallback_Device), Socket_Device);
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + ex.ToString()  +System.Environment.NewLine);
                Info.Update = true;
            }
        }
        private static void ConnectCallback_Device(IAsyncResult er)    //设备建立连接回调函数
        {
            try
            {
                Socket Socket_Device1 = (Socket)er.AsyncState;
                Socket_Device1.EndConnect(er);
                DelayMs(10);
                TData[0] = 0x3C;  //<
                TData[1] = Connect;
                TData[2] = 0x3E; //>
                SendMessage(3);
                ReceiveMessage();
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + ex.ToString() + System.Environment.NewLine);
                Info.Update = true;
            }
        }
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
                Information.Append("Error:" + ex.ToString() + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        private static void ReceiveCallback_Device(IAsyncResult er)    //接收数据回调函数
        {
            try
            {
                Socket Socket_Device1 = (Socket)er.AsyncState;
                DataLength = Socket_Device1.EndReceive(er);
                Receive.Received = true;
            }
            catch (Exception ex)
            {
                Information.Length = 0;
                Information.Append(System.DateTime.Now.ToString() + " ");
                Information.Append("Error:" + ex.ToString() + System.Environment.NewLine);
                Info.Update = true;
            }
        }
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
                Information.Append("Error:" + ex.ToString() + System.Environment.NewLine);
                Info.Update = true;
            }

        }
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
                Information.Append("Error:" + ex.ToString() + System.Environment.NewLine);
                Info.Update = true;
            }
        }
        void Info_Event(object sender, EventArgs e) //接收到设备数据触发事件
        {
            richTextBox1.AppendText(Information.ToString());       //显示错误详细信息
        }

        void Receive_Event(object sender, EventArgs e)    //接收到设备数据触发事件
        {
            if((RData[0]==0x3C)&&(RData[DataLength-1]==0x3E)) //判断数据包是否满足约定首尾格式
            {
                switch (RData[1])
                {

                    case 0x00 :  //建立连接
                        Information.Length = 0;
                        Information.Append(System .DateTime .Now .ToString ()+" ");
                        Information.Append("与设备 " + ConnectDevice.ToString() + " 成功建立连接" + System.Environment.NewLine);
                        Info.Update = true;
                        ReceiveMessage();
                        break;
                    case 0x01:
                        Socket_Device.Dispose();
                        Information.Length = 0;
                        Information.Append(System .DateTime .Now .ToString ()+" ");
                        Information.Append("与设备 " + ConnectDevice.ToString() + " 断开连接" + System.Environment.NewLine);
                        Info.Update = true;
                        break;
                    case 0x03:      //设备发送信息
                        Information.Length = 0;
                        Information.Append(System.DateTime.Now.ToString() + " ");
                        Information.Append("发送数据到设备 " + (RData[2] * 256 + RData[3]).ToString()+" 通道数：");

                        break;
                }
            }
        }
    
        //连接设备
        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboBox1.SelectedIndex == 0)
               TData[2]=1;
            if (comboBox1.SelectedIndex == 1)
               TData[2]=2;
            if (comboBox1.SelectedIndex == 2)
               TData[2]=3;
            if (comboBox1.SelectedIndex == 3)
               TData[2] = 4;
                
        }
        //主从选择
        private void button2_Click(object sender, EventArgs e)
        {
            TData[0] = 0x3C;  //<
            TData[1] = DisConnect;
            TData[2] = 0x3E; //>
            SendMessage(3);
            ReceiveMessage();
        }

        private void button3_Click(object sender, EventArgs e)
        {

        }
        public static void DelayMs(int MillSecond)   //延时函数，延时时间Ms
        {
            int Start = Environment.TickCount;
            while (System.Math.Abs(Environment.TickCount - Start) < MillSecond)
            {
                Application.DoEvents();
            }
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
