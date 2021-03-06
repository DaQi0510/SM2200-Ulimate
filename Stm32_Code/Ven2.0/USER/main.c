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
#include "Poll.h"

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

volatile u8 Device;                   //设备号
volatile u8  ConnectDevice[7];        //连接设备号
u32 ReceNum[7][19];                   //记录向各个设备的发送、接收次数
u16 ReceRoise[7][18];                 //记录各个通道的信号噪声
u8 RateFlag;                          //通信速率标记位
u16 RateTime[7];                      //接收发送一包数据时间
u8 Rates[7];                          //接收到通道数
volatile u8 DeviceScale;             //连接从属级别   0从 1主
volatile u16 ToDevice;               //要发送数据到的设备号
volatile u8 ReDevice;                //接收到数据的设备号
u8 DeviceInformation[13]={5,3660/256,3660%256,192,168,1,66,3670/256,3670%256,192,168,1,67};     //设备、IP号等信息

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
volatile u8 SM2200ReadFlag;          //标记程序正在读取数据


/************** W5500相关数据定义部分 ******************/
u8  ConnectState;     //设备连接状态				 
u8  ServiceIP[4]={192,168,1,1};
u16 ServicePort=2404;
u8 RJ45_1_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_1_IP[4]={192,168,1,66};
u8 RJ45_1_GateWay[4]={192, 168,1, 1};
u8 RJ45_1_SubNet[4]={255,255,255,0};
u16 RJ45_1_Loc_Potr=1266;
u8 RJ45_1_RData[1024];
u8 RJ45_1_WData[1024];
u16 RJ45_1_RLength;  //记录接收到数据长度
u8 RJ45_1_Connect;   //连接状态
u8 RJ45_1_ReceiveFlag;  //标记网口1是否接收到数据
u8 RJ45_1_Send;     //发送状态
u8 RJ45_1_DirIP[4]={192,168,1,66};   //对方服务器IP地址
u16 RJ45_1_Dir_Port;
u8 RJ45_2_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_2_IP[4]={192,168,1,66};
u8 RJ45_2_GateWay[4]={192, 168,1, 1};
u8 RJ45_2_SubNet[4]={255,255,255,0};
u16 RJ45_2_Loc_Potr=1266;
u8 RJ45_2_RData[1024];
u8 RJ45_2_WData[1024];
u16 RJ45_2_RLength;   //记录接收到数据长度
u8 RJ45_2_Connect;  //连接状态
u8 RJ45_2_ReceiveFlag;//标记网口2是否接收到数据
u8 RJ45_2_Send;     //发送状态
u16 RTR_Time=5000; //重新发送时间  Time=RTR_Time*100us
u8 RCR_Num=3;     //重新发送次数 
u16 NetRNum;
u16 NetTNum;
volatile u32 SendTem;       //临时发送通道
u8 RunMode;  //设备运行模式   轮询模式：1
//频点查找模式，自变量定义
u8 FConnectDevice;  //连接设备号
u8 FScale;          //连接设备号从属级别
u8 FStartVoltage;   //初始信号幅值
u8 FFlag[33];       //标记频点是否被测试过
u8 FReceNum[33];    //标记该频点接收次数
u8 FNosie[33];      //标记该频点噪声
u8 FFrequence[18];  //使用频点值


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
								0x0D--0x10 RJ45_1_DirIP(目标服务器地址)
								0x11--0x12 RJ45_1_Dir_Port（目标服务器端口号）
								0x13->主从模式  主模式1 从模式0：
								0x14->发送电压幅值
								0x15->0x1B载波通信设备连接号
								0x20->RunMode设备运行模式
*/

u32 receivenum[1000];
u8 volatile BER_Flag;
u32 trr;
u8 i;
int main(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);	   //延时函数初始化
	gpio_Init();           
  NetPower_On;         //网口电源
	SM2200Power_On;
	Voltage=5;
	delay_ms(10);
	AT24C02_Init();     
	delay_ms(10);
//	SetDeviceInformation(6,DeviceInformation,13);  //设置设备号、IP地址等信息
	GetDeviceInformation();
 if(Voltage<=MaxVoltage1)    //18个通道
		SendTem =0x3ffff;
	if((Voltage>MaxVoltage1)&&(Voltage<=MaxVoltage2))     //9个通道
		SendTem =0x15AAA;
	if((Voltage>MaxVoltage2)&&(Voltage<=MaxVoltage3))     //6个通道
		SendTem =0x12492;
	if(Voltage>MaxVoltage3)  //3个通道
		SendTem =0x8208;
	for(i=0;i<18;i++)
	{
		ChannelFrenquence[i]=35+3*i;
	}
	for(i=0;i<7;i++)  //计数单位清零
	{
		for(j=0;j<19;j++)
			ReceNum[i][j]=0;
	} 
  RateFlag =0;	
	SM2200_Init();
	delay_ms(10);
	TIM3_Init(10000,8399);
	RJ45_1_DirIP[0]=192;
	RJ45_1_DirIP[1]=168;
	RJ45_1_DirIP[2]=1;
	RJ45_1_DirIP[3]=71;
	RJ45_1_Dir_Port=3710;
//	uart_init(9600);
	
//	RJ45_1_Init();
//	RJ45_1_TCP_ServiceInit();
  
	RJ45_2_Init();
	RJ45_2_TCP_ServiceInit();
  delay_ms(10);
	RJ45_1_Init();
//	RJ45_1_TCP_ClientInit();
//	while(RJ45_1_TCP_ClientInit()==0)
//	{
//		
//		RJ45_1_Loc_Potr++;
//		delay_ms(1000);
//	}
	TIM4_Init();    //用于设备运行指示，1S进一次中断
	TIM5_Init();
	trr++;
	while(1)
	{
		switch(RunMode)
		{
			case 0x01:    //轮询方式
				Poll(3);
				break;
			case 0x03:   //查找频点模式
				break;
		}			
			
	}
}






