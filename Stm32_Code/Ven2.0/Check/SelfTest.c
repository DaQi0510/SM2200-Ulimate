#include "SelfTest.h"
#include "led.h"

extern volatile u8 Device;            //设备号

extern volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 ChannelSize[18];         //各通道数据长度
extern volatile u8 ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记

extern u32 receivenum[1000];
extern u8 BER_Flag;
extern u8 RJ45_1_WData[1024];
void SelfTest(u32 CheckNum)
{
	static u8 j;
	u8 i,l;
	u32 time;
	if(CheckNum==1)
	{
		u8 i=0;
	  SM2200_GPIO();
	  delayms(1);
	  SPI_SM2200 ();
	  RST_High ;
	  delayms(10);
	  RST_Low;	
	  delayms(10);
		RJ45_1_WData[0]='M';
		OfdmXcvrWrite(TRANCEIVER_CONFIG, 2,0x0005);
		OfdmXcvrWrite(INTERRUPT_MASK,2,0x0009);
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2, 0x04);
		
		for(i=0;i<NUMBER_OF_CLUSTERS;i++)
	  {
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    
			OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);	
			OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,10+5*i);
			OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	  }
		OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
		OfdmXcvrWrite(CLUSTER_SELECT,2,0);
		OfdmXcvrWrite(AGC_LEVELS,2,0x00);
		OfdmXcvrWrite(CLUSTER_SELECT,2,1);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
		OfdmXcvrWrite(CLUSTER_SELECT,2,2);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
		OfdmXcvrWrite(CLUSTER_SELECT,2,3);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
		OfdmXcvrWrite(CHANNEL_ENABLE,3,0x03ffff);
		OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0314);
		delayms(10);
		TIM3_Init(10000-1,840-1);
	}
	for(i=0;i<18;i++)
	{
		for(l=0;l<80;l++)
		{
			SM2200TxBuf[i][l]=i+l+j;
		}
	}
	j++;
	if(j>=150)
		j=0;
	for(i=0;i<18;i++)
	{
		ChannelSend |=1<<i;
		ChannelSize [i]=35;
		ChannelType [i]=0;
	}
	SM2200_Send();
	ChannelSend=0;
//	while(SM2200ReceiveFalg!=1);
//	TIM3->CNT=0;
//	while(ChannelReceive!=0x03ffff)
//	{
//		time=TIM3->CNT;
//		if(time>1000)
//			break;
//	}
//	if(ChannelReceive==0x03fffd)
//	{
//		LED5 =!LED5;
//	}
	if(CheckNum<1000)
	receivenum[CheckNum]=ChannelReceive;
	ChannelReceive=0;
	SM2200ReceiveFalg=0;
}
