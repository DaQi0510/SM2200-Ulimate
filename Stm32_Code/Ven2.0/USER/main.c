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
//   ���������ⲿ�жϽ��п��� ,�ⲿ�жϴ�������LED.C�ļ���

/*******************�豸����****************************/
volatile u8 Command[74];             //������������ 

volatile u8 Device;                  //�豸��
volatile u8 ConnectDevice[7];        //�����豸��
volatile u8 DeviceScale;             //���Ӵ�������   0�� 1��
volatile u16 ToDevice;               //Ҫ�������ݵ����豸��
volatile u16 ReDevice;               //���յ����ݵ��豸��
u8 DeviceInformation[13]={5,3660/256,3660%256,192,168,1,66,3670/256,3670%256,192,168,1,67};     //�豸��IP�ŵ���Ϣ

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
volatile u8 Voltage;                 //��¼��ѹ��ֵ

u8 Check[50]={'d','a','q','i',0,5,1,0,6,6,
               12,16,18,21,11,3,15,78,79,80,  
               103,109,200,37,43,3,25,68,78,90, 
               12,16,18,21,11,3,15,78,79,80,	
	             103,109,200,37,43,3,25,68,78,90};

/************** W5500������ݶ��岿�� ******************/
u8  ConnectState;     //�豸����״̬				 
u8  ServiceIP[4]={192,168,1,1};
u16 ServicePort=2404;
u8 RJ45_1_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_1_IP[4]={192,168,1,66};
u8 RJ45_1_GateWay[4]={192, 168,1, 1};
u8 RJ45_1_SubNet[4]={255,255,255,0};
u16 RJ45_1_Loc_Potr=1266;
u8 RJ45_1_RData[1024];
u8 RJ45_1_WData[1024];
u16 RJ45_1_RLength;  //��¼���յ����ݳ���
u8 RJ45_1_Connect;  //����״̬
u8 RJ45_1_ReceiveFlag;  //�������1�Ƿ���յ�����
u8 RJ45_1_Send;     //����״̬
u8 RJ45_1_DirIP[4]={192,168,1,66};   //�Է�������IP��ַ
u16 RJ45_1_Dir_Port;
u8 RJ45_2_MAC[6]={0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
u8 RJ45_2_IP[4]={192,168,1,66};
u8 RJ45_2_GateWay[4]={192, 168,1, 1};
u8 RJ45_2_SubNet[4]={255,255,255,0};
u16 RJ45_2_Loc_Potr=1266;
u8 RJ45_2_RData[1024];
u8 RJ45_2_WData[1024];
u16 RJ45_2_RLength;   //��¼���յ����ݳ���
u8 RJ45_2_Connect;  //����״̬
u8 RJ45_2_ReceiveFlag;//�������2�Ƿ���յ�����
u8 RJ45_2_Send;     //����״̬
u16 RTR_Time=5000; //���·���ʱ��  Time=RTR_Time*100us
u8 RCR_Num=3;     //���·��ʹ��� 
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
								0x0D--0x10 RJ45_1_DirIP(Ŀ���������ַ)
								0x11--0x12 RJ45_1_Dir_Port��Ŀ��������˿ںţ�
								0x13->����ģʽ  ��ģʽ1 ��ģʽ0��
								0x14->���͵�ѹ��ֵ
								0x15->0x1B�ز�ͨ���豸���Ӻ�
*/

u32 receivenum[1000];
u8 volatile BER_Flag;
u32 trr;
u8 i;
int main(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);	   //��ʱ������ʼ��
	gpio_Init();           
  NetPower_On;         //���ڵ�Դ
	SM2200Power_On;
	Voltage=5;
	delay_ms(10);
	AT24C02_Init();     
	delay_ms(10);
//	SetDeviceInformation(6,DeviceInformation,13);  //�����豸�š�IP��ַ����Ϣ
	GetDeviceInformation();
	for(i=0;i<18;i++)
	{
		ChannelFrenquence[i]=20+4*i;
	}
	SM2200_Init();
	delay_ms(10);
	TIM3_Init(10000,8399);
	RJ45_1_DirIP[0]=192;
	RJ45_1_DirIP[1]=168;
	RJ45_1_DirIP[2]=1;
	RJ45_1_DirIP[3]=85;
	RJ45_1_Dir_Port=3590 + 20 * 7;
//	uart_init(9600);
	
//	RJ45_1_Init();
//	RJ45_1_TCP_ServiceInit();
  
	RJ45_2_Init();
	RJ45_2_TCP_ServiceInit();
  delay_ms(10);
//	RJ45_1_Init();
//	RJ45_1_TCP_ClientInit();
	TIM4_Init();    //�����豸����ָʾ��1S��һ���ж�
	trr++;
	while(1)
	{
		OPTest(trr);
	}
}






