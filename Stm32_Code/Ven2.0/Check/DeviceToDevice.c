#include "DeviceToDevice.h"
extern volatile u8 Device;            //设备号

extern volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 ChannelSize[18];         //各通道数据长度
extern volatile u8 ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记

void DeviceToDevice(u32 CheckNum)
{
	u8 i;
}
