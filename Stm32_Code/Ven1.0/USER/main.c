#include "stm32f4xx.h"
#include "led.h"
#include "delay.h"
#include "lcd.h"
#include "sm2200.h"
#include "time.h"
#include "W5200.h"
#include "AT24C02.h"
#include "SelfTest.h"
#include "OPTest.h"
#include "NetCheck.h"
#include "Simulation.h"
#include "uart.h"	
#include "SendReceive.h"
#include "wwdg.h"

/**********   SM2200最终版本    **************/
/**********建立时间：2015年7月26日************/
/**********    Author:大岐       *************/

//***************引脚介绍********************

//**************SM2200部分*******************
//  PD0->SPI_NSS  PC10->SPI_SCK   PC11->SPI_MISO   PC12->SPI_MOSI  PA1->RST  PA0->INT

//*************电源控制部分******************
//    PG7高电平打开网口供电部分   PA11 SM2200供电部分

//*************辅助控制外设******************
//   PE1->小灯1   PE0->小灯2  PB7->小灯5  PB9->小灯3  PB8->小灯4

//**************RJ45-1部分*******************
//    PB12->SPI_NSS  PB13->SPI_SCK   PB14->SPI_MISO   PB15->SPI_MOSI   PE14->INT   PE13->PWDN  PE12->RST

//**************RJ45-2部分*******************
//    PA4->SPI_NSS  PA5->SPI_SCK   PA6->SPI_MISO   PA7->SPI_MOSI   PC5->INT   PA3->PWDN     PA2->RST

//**************  说明     ******************
//   按键、小灯、电源控制部分初始化函数均在 LED.C文件下
//   小灯、电源控制部分、液晶引脚初始化均在gpio_Init()函数下
//   小灯控制部分   LED_Red=1；（红灯亮）   LED_Blue=1；（蓝灯亮） 
//   按键采用外部中断进行控制 ,外部中断处理函数在LED.C文件下

/*******************设备部分****************************/
volatile u8 Command[74];             //主机控制命令 

volatile u8 Device;                  //设备号
volatile u8 ConnectDevice;           //连接设备号
volatile u8 DeviceScale;             //连接从属级别   0从 1主
volatile u16 ToDevice;               //要发送数据到的设备号
volatile u16 ReDevice;               //接收到数据的设备号
u8 DeviceInformation[13]={4,3660/256,3660%256,192,168,1,66,3670/256,3670%256,192,168,1,67};     //设备、IP号等信息

/*******************SM2200部分****************************/
volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
volatile u8 ChannelFrenquence[18];   //18个通道频点数组
volatile u8 ChannelSize[18];         //各通道数据长度
volatile u8 ChannelType[18];         //各通道发送模式
volatile u32 ChannelSend;            //标记发送的通道
volatile u32 ChannelReceive;         //标记接收的通道
volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记
volatile u16 Noise[18];              //记录通道噪声
volatile u8  ShakeChannel[18];       //记录通道噪声
volatile u8 Voltage;                 //记录电压幅值

u8 Check[50]={'d','a','q','i',0,5,1,0,6,6,
               12,16,18,21,11,3,15,78,79,80,  
               103,109,200,37,43,3,25,68,78,90, 
               12,16,18,21,11,3,15,78,79,80,	
	             103,109,200,37,43,3,25,68,78,90};

/************** W5200相关数据定义部分 ******************/
u8  ConnectState;     //设备连接状态
u8 RJ45_1_Mode;		    //记录网口模式，0-服务器 1-客户端					 
u8  ServiceIP[4]={192,168,1,71};
u16 ServicePort=3710;
u8 RJ45_1_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_1_IP[4]={192,168,1,66};
u8 RJ45_1_GateWay[4]={192, 168,1, 4};
u8 RJ45_1_SubNet[4]={255,255,255,0};
u16 RJ45_1_Loc_Potr=1266;
u8 RJ45_1_RData[1024];
u8 RJ45_1_WData[1024];
u8 RJ45_1_Connect;  //连接状态
u8 RJ45_1_ReceiveFlag;  //标记网口1是否接收到数据
u8 RJ45_1_Send;     //发送状态
u8 RJ45_2_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_2_IP[4]={192,168,1,66};
u8 RJ45_2_GateWay[4]={192, 168,1, 4};
u8 RJ45_2_SubNet[4]={255,255,255,0};
u16 RJ45_2_Loc_Potr=1266;
u8 RJ45_2_RData[1024];
u8 RJ45_2_WData[1024];
u8 RJ45_2_Connect;  //连接状态
u8 RJ45_2_ReceiveFlag;//标记网口2是否接收到数据
u8 RJ45_2_Send;     //发送状态
u16 RTR_Time=5000; //重新发送时间  Time=RTR_Time*100us
u8 RCR_Num=3;   //重新发送次数 
u16 NetRNum;
u16 NetTNum;

volatile u8 CommandFlag;

volatile u8 Netflag;   //用于指示当前网络所处状态

u8 W5200_Device1_Connect=Fail;//连接状态
u8 W5200_Device1_Send=Ok;     //发送状态

u16 W5200_Device1_ReadAddress=0;
u8 ShakeHandFlag=0;
u16 Flag=0;
u8 r,m;
u16 j,l;
u16 Start_Address;
u8 Len;
/************** 规约变量部分 ******************/
u8 Flag101;
u8 Flag104;

u8 M101[5];
u8 M104_1[6]={0x68,0x04,0x43,0x00,0x00,0x00};
u8 M104_2[6]={0x68,0x04,0x07,0x00,0x00,0x00};
u8 M104_3[16]={0x68,0x0E,0x00,0x00,0x00,0x00,0x64,0x01,0x06,0x00,0x01,0x00,0x00,0x00,0x00,0x14};
/************** AT24C02设定 ******************/
/*------------------|--------------------
								0x00->设备号
								0x01->RJ45_1端口号高八位
								0x02->RJ45_1端口号低八位
								0x03->RJ45_1IP号
								0x04->RJ45_1IP号
								0x05->RJ45_1IP号
								0x06->RJ45_1IP号
								0x07->RJ45_2端口号高八位
								0x08->RJ45_2端口号低八位
								0x09->RJ45_2IP号
								0x0A->RJ45_2IP号
								0x0B->RJ45_2IP号
								0x0C->RJ45_2IP号
*/

u32 receivenum[1000];
u8 volatile BER_Flag;
u32 trr;
u8 i;
int main(void)
{
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);	   //延时函数初始化
	gpio_Init();
  NetPower_On;         //网口电源
	SM2200Power_On ;
	Voltage=5;
	delay_ms(10);
	AT24C02_Init();     
	delay_ms(10);
	GetDeviceInformation();
  delay_ms (10);
	for(i=0;i<18;i++)
	{
		ChannelFrenquence[i]=20+4*i;
	}
	SM2200_Init();
	delay_ms(10);
	TIM3_Init(10000,8399);
	uart_init(9600);
	
	RJ45_1_Connect=0;
	RJ45_2_Connect=0;
	RJ45_1_ReceiveFlag=0;
	i=1;
  while(i==1)
	{
		RJ45_1_Init();
		delay_ms(100);
		i=RJ45_1_TCP_ClientInit();
	}
	
//	RJ45_2_Init();
//	delay_ms(100);
//	RJ45_2_TCP_Init();
	trr++;
	while(1)
	{
		if(Device==4)
		{
		}
		delay_ms(100);
	}
//		if(Device==3)
//	  {
//			ToDevice=1;
//			ChannelSend =0x3ffff;
//			for(i=0;i<18;i++)
//			{
//				SM2200TxBuf[i][0]=ToDevice/256;
//				SM2200TxBuf[i][1]=ToDevice%256;
//				for(l=0;l<18;l++)
//				{
//					SM2200TxBuf[i][2*l+2]=l;
//					SM2200TxBuf[i][2*l+3]=ChannelFrenquence[i];
//				}
//				SM2200TxBuf[i][38]=Voltage ;
//				SM2200TxBuf[i][39]=ChannelSend/65536;
//				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
//				SM2200TxBuf[i][41]=ChannelSend%256;	
//        SM2200TxBuf[i][42]=Device/256;		
//				SM2200TxBuf[i][43]=Device%256;
//				ChannelSize[i]=45;              //数据长度          
//				ChannelType[i]=0;               //传输类型				
//			}
//	  		delayms(1000);
//  			Message_Send ();
//			SM2200_Send();
//			delayms(1000);
//      if(SM2200ReceiveFalg!=0)
//			{
//				SM2200ReceiveFalg=0;
//				LED5 =!LED5;
//				Message_Rece();
//				ChannelReceive=0;
//			}
//		}
//		if(Device==1)
//		{
//			if(SM2200ReceiveFalg!=0)
//			{
//				delay_ms(30);
//				ChannelReceive=0;
//				SM2200ReceiveFalg=0;
//				LED3 =!LED3 ;
//				ToDevice=3;
//				ChannelSend =0x3ffff;
//				for(i=0;i<18;i++)
//				{
//					SM2200TxBuf[i][0]=ToDevice/256;
//					SM2200TxBuf[i][1]=ToDevice%256;
//					for(l=0;l<18;l++)
//					{
//						SM2200TxBuf[i][2*l+2]=l;
//						SM2200TxBuf[i][2*l+3]=ChannelFrenquence[i];
//					}
//					SM2200TxBuf[i][38]=Voltage ;
//					SM2200TxBuf[i][39]=ChannelSend/65536;
//					SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
//					SM2200TxBuf[i][41]=ChannelSend%256;	
//					SM2200TxBuf[i][42]=Device/256;		
//					SM2200TxBuf[i][43]=Device%256;
//					ChannelSize[i]=45;              //数据长度          
//					ChannelType[i]=0;               //传输类型				
//				}
//				Message_Send ();
//				SM2200_Send();
//			}		
//		}
//	}
//			TIM3->CNT=0;
//			SM2200ReceiveFalg=0;
//			while(TIM3->CNT<6000)    //延时600ms
//			{
//				if(SM2200ReceiveFalg!=0)
//					break;
//			}
//			TIM3->CNT=0;
//			while(TIM3->CNT<500);   //延时50ms
//		}
//		if(Device==1)
//		{
//			ToDevice=3;
//			TIM3->CNT=0;
//			SM2200ReceiveFalg=0;
//			while(1)    //延时600ms
//			{
//				if(SM2200ReceiveFalg!=0)
//					break;
//			}
//			TIM3->CNT=0;
//			while(TIM3->CNT<500);   //延时50ms
//			ChannelSend =0x3ffff;
//			for(i=0;i<18;i++)
//			{
//				SM2200TxBuf[i][0]=ToDevice/256;
//				SM2200TxBuf[i][1]=ToDevice%256;
//				for(l=0;l<18;l++)
//				{
//					SM2200TxBuf[i][2*l+2]=l;
//					SM2200TxBuf[i][2*l+3]=ChannelFrenquence[i];
//				}
//				SM2200TxBuf[i][38]=Voltage ;
//				SM2200TxBuf[i][39]=ChannelSend/65536;
//				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
//				SM2200TxBuf[i][41]=ChannelSend%256;	
//        SM2200TxBuf[i][42]=Device/256;		
//				SM2200TxBuf[i][43]=Device%256;
//				ChannelSize[i]=45;              //数据长度          
//				ChannelType [i]=0;              //传输类型				
//			}
//			SM2200_Send();
//			Message_Send ();
//		}

//	}
	

	while(1)
	{
		LED1=!LED1 ;
		LED2=!LED2 ;
		LED3=!LED3 ;
		LED4=!LED4 ;
		LED5=!LED5 ;
		for(i=0;i<18;i++)
		{
			for(j=0;j<35;j++)
			{
				SM2200TxBuf[i][j]=100;
			}
			ChannelSize[i]=45;              //数据长度          
			ChannelType[i]=0;               //传输类型
			ChannelSend|=1<<i;              //发送通道数
		}
		SM2200_Send();
		delay_ms(1000);
	}

	
		
		RJ45_1_Write(M104_1,6);
		while(RJ45_1_ReceiveFlag==0);
		RJ45_1_ReceiveFlag=0;
		delay_ms(10000);
		
		RJ45_1_Write(M104_2,6);
		while(RJ45_1_ReceiveFlag==0);
		RJ45_1_ReceiveFlag=0;
		delay_ms(10000);

		RJ45_1_Write(M104_3,16);
		while(RJ45_1_ReceiveFlag==0);
		RJ45_1_ReceiveFlag=0;
		delay_ms(10000);
		
		

  while(1)
	{
		SendReceive();
	}
  
	while(1)
	{
		FindChatChannel();
	}

	
}






