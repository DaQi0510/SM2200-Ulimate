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

/**********   SM2200���հ汾    **************/
/**********����ʱ�䣺2015��7��26��************/
/**********    Author:���       *************/

//***************���Ž���********************

//**************SM2200����*******************
//  PD0->SPI_NSS  PC10->SPI_SCK   PC11->SPI_MISO   PC12->SPI_MOSI  PA1->RST  PA0->INT

//*************��Դ���Ʋ���******************
//    PG7�ߵ�ƽ�����ڹ��粿��   PA11 SM2200���粿��

//*************������������******************
//   PE1->С��1   PE0->С��2  PB7->С��5  PB9->С��3  PB8->С��4

//**************RJ45-1����*******************
//    PB12->SPI_NSS  PB13->SPI_SCK   PB14->SPI_MISO   PB15->SPI_MOSI   PE14->INT   PE13->PWDN  PE12->RST

//**************RJ45-2����*******************
//    PA4->SPI_NSS  PA5->SPI_SCK   PA6->SPI_MISO   PA7->SPI_MOSI   PC5->INT   PA3->PWDN     PA2->RST

//**************  ˵��     ******************
//   ������С�ơ���Դ���Ʋ��ֳ�ʼ���������� LED.C�ļ���
//   С�ơ���Դ���Ʋ��֡�Һ�����ų�ʼ������gpio_Init()������
//   С�ƿ��Ʋ���   LED_Red=1�����������   LED_Blue=1������������ 
//   ���������ⲿ�жϽ��п��� ,�ⲿ�жϴ���������LED.C�ļ���

/*******************�豸����****************************/
volatile u8 Command[74];             //������������ 

volatile u8 Device;                  //�豸��
volatile u8 ConnectDevice;           //�����豸��
volatile u8 DeviceScale;             //���Ӵ�������   0�� 1��

u8 DeviceInformation[13]={4,3660/256,3660%256,192,168,1,66,3670/256,3670%256,192,168,1,67};     //�豸��IP�ŵ���Ϣ

/*******************SM2200����****************************/
volatile u8 SM2200TxBuf[18][128];    //18��ͨ���������ݰ�����
volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
volatile u8 ChannelSize[18];         //��ͨ�����ݳ���
volatile u8 ChannelType[18];         //��ͨ������ģʽ
volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
volatile u8 SM2200ReceiveFalg;       //��������ʱ���ձ��
volatile u16 Noise[18];              //��¼ͨ������
volatile u8  ShakeChannel[18];       //��¼ͨ������
u8 Voltage;                          //��¼��ѹ��ֵ

u8 Check[50]={'d','a','q','i',0,5,1,0,6,6,
               12,16,18,21,11,3,15,78,79,80,  
               103,109,200,37,43,3,25,68,78,90, 
               12,16,18,21,11,3,15,78,79,80,	
	             103,109,200,37,43,3,25,68,78,90};

/************** W5200������ݶ��岿�� ******************/
u8  ConnectState;     //�豸����״̬
u8 RJ45_1_Mode;		    //��¼����ģʽ��0-������ 1-�ͻ���					 
u8  ServiceIP[4]={192,168,1,1};
u16 ServicePort=2404;
u8 RJ45_1_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_1_IP[4]={192,168,1,66};
u8 RJ45_1_GateWay[4]={192, 168,1, 4};
u8 RJ45_1_SubNet[4]={255,255,255,0};
u16 RJ45_1_Loc_Potr=1266;
u8 RJ45_1_RData[1024];
u8 RJ45_1_WData[1024];
u8 RJ45_1_Connect;  //����״̬
u8 RJ45_1_ReceiveFlag;  //�������1�Ƿ���յ�����
u8 RJ45_1_Send;     //����״̬
u8 RJ45_2_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_2_IP[4]={192,168,1,66};
u8 RJ45_2_GateWay[4]={192, 168,1, 4};
u8 RJ45_2_SubNet[4]={255,255,255,0};
u16 RJ45_2_Loc_Potr=1266;
u8 RJ45_2_RData[1024];
u8 RJ45_2_WData[1024];
u8 RJ45_2_Connect;  //����״̬
u8 RJ45_2_ReceiveFlag;//�������2�Ƿ���յ�����
u8 RJ45_2_Send;     //����״̬
u16 RTR_Time=5000; //���·���ʱ��  Time=RTR_Time*100us
u8 RCR_Num=3;   //���·��ʹ��� 
u16 NetRNum;
u16 NetTNum;

volatile u8 CommandFlag;

volatile u8 Netflag;   //����ָʾ��ǰ��������״̬

u8 W5200_Device1_Connect=Fail;//����״̬
u8 W5200_Device1_Send=Ok;     //����״̬

u16 W5200_Device1_ReadAddress=0;
u8 ShakeHandFlag=0;
u16 Flag=0;
u8 r,m;
u16 j,l;
u16 Start_Address;
u8 Len;
/************** ��Լ�������� ******************/
u8 Flag101;
u8 Flag104;

u8 M101[5];
u8 M104_1[6]={0x68,0x04,0x43,0x00,0x00,0x00};
u8 M104_2[6]={0x68,0x04,0x07,0x00,0x00,0x00};
u8 M104_3[16]={0x68,0x0E,0x00,0x00,0x00,0x00,0x64,0x01,0x06,0x00,0x01,0x00,0x00,0x00,0x00,0x14};
/************** AT24C02�趨 ******************/
/*------------------|--------------------
								0x00->�豸��
								0x01->RJ45_1�˿ںŸ߰�λ
								0x02->RJ45_1�˿ںŵͰ�λ
								0x03->RJ45_1IP��
								0x04->RJ45_1IP��
								0x05->RJ45_1IP��
								0x06->RJ45_1IP��
								0x07->RJ45_2�˿ںŸ߰�λ
								0x08->RJ45_2�˿ںŵͰ�λ
								0x09->RJ45_2IP��
								0x0A->RJ45_2IP��
								0x0B->RJ45_2IP��
								0x0C->RJ45_2IP��
*/

u32 receivenum[1000];
u8 volatile BER_Flag;
u32 trr;
u8 i;
int main(void)
{

	delay_init(168);	   //��ʱ������ʼ��
	gpio_Init();
  NetPower_On;         //���ڵ�Դ
	SM2200Power_On ;
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
	
	RJ45_1_Init();
	delay_ms(100);
	RJ45_1_TCP_ServiceInit();
	
	RJ45_2_Init();
	delay_ms(100);
	RJ45_2_TCP_Init();
	
	trr++;
	while(1)
	{
		SelfTest(trr);
		trr++;
		LED1=!LED1 ;
		LED2=!LED2 ;
		LED3=!LED3 ;
		LED4=!LED4 ;
		LED5=!LED5 ;
		delay_ms(500);
	}
//	while(1)
//	{
//		USART_Send(RJ45_1_MAC,3);
//	}
//	if(Device==4)      //���������
//	{
//		RJ45_1_IP[0]=192;
//		RJ45_1_IP[1]=168;
//		RJ45_1_IP[2]=1;
//		RJ45_1_IP[3]=1;
//		RJ45_1_Loc_Potr=2404;
//		RJ45_1_Init();
//		delay_ms(100);
//		RJ45_1_TCP_ServiceInit();
//		delay_ms(100);
//	}
//	if(Device==4)      //����վ����
//	{
//		RJ45_1_Init();
//		delay_ms(100);
//		RJ45_1_TCP_ClientInit();
//		delay_ms(100);
//	
//		while(ConnectState)      //�ȴ����ӵ���վ������
//		{
//			LED5=!LED5 ;
//			RJ45_1_TCP_ClientInit();
//			delay_ms(100);
//		}
//		
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
			ChannelSize[i]=45;              //���ݳ���          
			ChannelType[i]=0;               //��������
			ChannelSend|=1<<i;              //����ͨ����
		}
		SM2200_Send();
		delay_ms(1000);
	}
//	while(1)
//	{
//		
//		SendReceivec();
//		delay_ms(10);
//	}
//	while(1)
//	{
//		USART_Send(M101,5);
//	}
//	if(Device==2)
//	{
	
		
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
		
		
//		while(RJ45_1_Connect==0);
////	}
////	if(Device==4)
////	{
//			RJ45_2_Init();
//			delay_ms(100);
//			RJ45_2_TCP_Init();
//			while(RJ45_2_Connect==0)
//			{
//				LED4 =!LED4 ;
//				delay_ms (100);
//			}
////  }
  while(1)
	{
		SendReceive();
	}
  
	while(1)
	{
		FindChatChannel();
	}
//		OPTest(trr);
//		trr++;
//		delay_ms(1000);
		
//		if(Device==4)
//		{
//		Simulation(4,1);
//		delay_ms(5000);
//		}
//		if(Device==4)
//		{
//			Simulation(4,1);
//		}
////		RJ45_1_WData[1]=i;
//		RJ45_1_WData[2]=i+5;
//		RJ45_1_WData[3]=i+2;
//	}
	
}





