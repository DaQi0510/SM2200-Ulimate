using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WindowsFormsApplication1
{
    class G
    {
        //数据包统计部分变量定义
        public static Int32[] StatisPacket = new Int32[18];   //通道数据包统计
        public static Int32 TolPacket;                        //总共发送包数
        public static float[] PacketRate = new float[18];     //数据包接收率
        public static byte[] Frequence = new byte[18];        //数据包接收率
        public static byte Device;                            //记录显示的设备号
        public static byte ConnectDevice;                     //记录网络连接的设备号
        public static byte PacketNum=88;                      //记录每个通道的数据个数
        public static float Rates;                            //记录通信速率
        public static byte StartVoltage;                      //记录信道查找初始电压
        public static byte ConnectToDevice;                   //记录信道查找连接设备
        public static byte FScale;                            //记录信道查找从属级别 1主模式 0从模式  
        public static Int32[] Noise = new Int32[18];          //记录信道噪声
        //SM2200通信频点(刨除前两个频点)
        public static float[] Frequences={14.6f, 19.5f, 24.4f, 29.3f, 34.2f, 39.1f, 43.9f, 48.8f, 53.7f, 58.6f,
                                          63.5f, 68.4f, 73.2f, 78.1f, 83.0f, 87.9f, 92.8f, 97.7f, 102.5f, 107.4f,
                                          112.3f, 117.2f, 122.1f, 127.0f, 131.8f, 136.7f, 141.6f, 146.5f, 151.4f, 156.3f,
                                          161.1f, 166.0f, 170.9f, 175.8f, 180.7f, 185.5f, 190.4f, 195.3f, 200.2f, 205.1f,
                                          210.0f, 214.8f, 219.7f, 224.6f, 229.5f, 234.4f, 239.3f, 244.1f, 249.0f, 253.9f,
                                          258.8f, 263.7f, 268.6f, 273.4f, 278.3f, 283.2f, 288.1f, 293.0f, 297.9f, 302.7f,
                                          307.6f, 312.5f, 317.4f, 322.3f, 327.1f, 332.0f, 336.9f, 341.8f, 346.7f, 351.6f,
                                          356.4f, 361.3f, 366.2f, 371.1f, 376.0f, 380.9f, 385.7f, 390.6f, 395.5f, 400.4f,
                                          405.3f, 410.2f, 415.0f, 419.9f, 424.8f, 429.7f, 434.6f, 439.5f, 444.3f, 449.2f,
                                          454.1f, 459.0f, 463.9f, 468.8f, 473.6f, 478.5f, 483.4f, 488.3f, 493.2f, 498.0f };
    }
}
