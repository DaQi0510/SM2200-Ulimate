using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WindowsFormsApplication1
{
    class G
    {
        public static Int32[] StatisPacket = new Int32[18];   //通道数据包统计
        public static Int32 TolPacket;                        //总共发送包数
        public static float[] PacketRate = new float[18];     //数据包接收率
    }
}
