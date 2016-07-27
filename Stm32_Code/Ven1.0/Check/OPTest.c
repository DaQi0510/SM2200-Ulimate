#include "OPTest.h"
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
extern volatile u16 Noise[18]; 
void OPTest(u32 CheckNum)
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
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2, 0x03);
		
		for(i=0;i<NUMBER_OF_CLUSTERS ;i++)
	  {
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    
			OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);	
			OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,13+5*i);
			OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	  }
		OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
		OfdmXcvrWrite(CLUSTER_SELECT,2, 0);
		OfdmXcvrWrite(AGC_LEVELS,2,0x00);
		OfdmXcvrWrite(CLUSTER_SELECT,2,1);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
		OfdmXcvrWrite(CLUSTER_SELECT,2,2);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
		OfdmXcvrWrite(CLUSTER_SELECT,2,3);
		OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
		OfdmXcvrWrite(CHANNEL_ENABLE,3,0x03ffff);
		OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0310);
		delayms(10);
		TIM3_Init(10000-1,8400-1);
	}
	if(Device==3)
	{
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
		for(i=15;i<18;i++)
		{
			ChannelSend |=1<<i;
			ChannelSize [i]=35;
			ChannelType [i]=0;
		}
		SM2200_Send ();
		ChannelSend=0;
	  LED1 =!LED1 ;
		delay_ms(500);
	}
	if(Device==1)
	{

		if(SM2200ReceiveFalg==1)
		{
			TIM3->CNT=0;
			while(ChannelReceive!=0x03ffff)
			{
				time=TIM3->CNT;
				if(time>3000)
					break;
			}
			if(ChannelReceive==0x03ffff)
			{
				GPIO_ToggleBits(GPIOE,GPIO_Pin_1);
			}
			for(i=0;i<18;i++)
			{
				OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
			Noise[i]=OfdmXcvrRead(CARRIER_NOISE,2);
			}
			if(BER_Flag==1)
			{
				RJ45_1_WData[1]=(ChannelReceive&0xff0000)>>16;
				RJ45_1_WData[2]=(ChannelReceive&0x00ff00)>>8;
				RJ45_1_WData[3]=ChannelReceive&0x0000ff;
				RJ45_1_Write(RJ45_1_WData,8);
			}
			receivenum[j]=ChannelReceive;
			j++;
			ChannelReceive=0;
			SM2200ReceiveFalg=0;
		}
	}
}
