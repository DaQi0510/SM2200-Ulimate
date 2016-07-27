#include "W5200.h"
#include "led.h"
#include "wwdg.h"
#include "sm2200.h"
extern u8  ConnectState;       //�豸����״̬

extern volatile u8 ConnectDevice;     //�����豸��
extern volatile u8 DeviceScale;       //���Ӵ�������   0�� 1��

extern volatile u8 Command[74];       //������������
extern u8 ServiceIP[4];
extern u16 ServicePort;
extern u8 RJ45_1_MAC[6];
extern u8 RJ45_1_IP[4];
extern u8 RJ45_1_GateWay[4];
extern u8 RJ45_1_SubNet[4];
extern u16 RJ45_1_Loc_Potr;
extern u8 RJ45_1_RData[1024];
extern u8 RJ45_1_WData[1024];
extern u16 RJ45_1_RLength;
extern u8 RJ45_1_Connect;  //����״̬
extern u8 RJ45_1_ReceiveFlag;
extern u16 RJ45_1_Send;     //����״̬
extern u8 RJ45_2_MAC[6];
extern u8 RJ45_2_IP[4];
extern u8 RJ45_2_GateWay[4];
extern u8 RJ45_2_SubNet[4];
extern u16 RJ45_2_Loc_Potr;
extern u8 RJ45_2_RData[1024];
extern u8 RJ45_2_WData[1024];
extern u8 RJ45_2_RLength;
extern u8 RJ45_2_Connect;     //����״̬
extern u8 RJ45_2_ReceiveFlag; //�������2�Ƿ���յ�����
extern u8 RJ45_2_Send;        //����״̬
extern u8 Dest_IP[4] ;
extern u16 RTR_Time; //���·���ʱ��
extern u8 RCR_Num;   //���·��ʹ��� 
u8 W5200_Rx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Rx_Size[i])K And SUM=16K
u8 W5200_Tx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Tx_Size[i])K And SUM=16K
extern volatile u8 CommandFlag;
u16 RJ45_2_DataLength;
extern u8 ShakeHands[7];
extern u8 Flag;
extern u8 Len;
extern volatile u8 Netflag;   //����ָʾ��ǰ��������״̬
extern volatile u8 BER_Flag;

extern volatile u32 ChannelSend;     //��Ƿ��͵�ͨ��
extern volatile u32 ChannelReceive;  //��ǽ��յ�ͨ��
extern volatile u8 ChannelFrenquence[18];//18��ͨ��Ƶ������
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern u8 Voltage;                   //��¼��ѹ��ֵ
extern volatile u16 ToDevice;        //Ҫ���͵����ݵ��豸��  
extern volatile u16 ReDevice;        //���յ����ݵ��豸��

u8 WriteTem[2];
u8 ReadTem[2];
/********************RJ45_1����*****************************/
/*
--------------|-------------
        RST  <->  PE12
				INT  <->  PE14
				CS   <->  PB12
		    CLK  <->  PB13
		    MISO <->  PB14
		    MOSI <->  PB15			
  */
void RJ45_1_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	//RST
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOEʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOE,GPIO_Pin_12);//���ߣ�����λ
	
	
	//SPI2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//ʹ��SPI2ʱ��
	
	//PB12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);//���ߣ�ʧ��SPI����
	
	//���Ÿ���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PB13~15���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); //PB13����Ϊ SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); //PB14����Ϊ SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); //PB15����Ϊ SPI2
	
	SPI_I2S_DeInit(SPI2);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	
	SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���	
	SPI_Cmd(SPI2, ENABLE);
	
	//INT
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOEʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;//INT��ӦIO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource14);//PE14 ���ӵ��ж���14
	
	/* ����EXTI_Line14 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;//LINE10
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ش��� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE10
	EXTI_Init(&EXTI_InitStructure);//����
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//�ⲿ�ж�10
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//����	
}

void RJ45_1_Reset(void)
{
	uint16_t i,j;
	RJ45_1_RST_Low ;
	for(i=0;i<25000;i++);
	RJ45_1_RST_High ;
	for(i=0;i<5000;i++)
	{
		for(j=0;j<25000;j++);
	}
}

void RJ45_1_Init(void)
{
	u8 i;
	RJ45_1_GPIO_Init();
	RJ45_1_Reset();
	//����MAC ADDRESS
	RJ45_1_Write_Buf(SHAR0,RJ45_1_MAC,6);
	//����Subnet Mask ADDRESS
	RJ45_1_Write_Buf(SUBR0,RJ45_1_SubNet,4);
	//����GateWay ADDRESS
	RJ45_1_Write_Buf(GAR0,RJ45_1_GateWay,4);
	//����Local IP ADDRESS
	RJ45_1_Write_Buf(SIPR0,RJ45_1_IP,4);

	//�������·���ʱ���Լ��ط�����
	WriteTem[0]=RTR_Time/256;
	WriteTem[1]=RTR_Time%256;
	RJ45_1_Write_Buf(RTR0,WriteTem,2);
	RJ45_1_Write_Register(WIZ_RCR,RCR_Num);
	//���ø�Socket���͡�����������ݰ�
	for(i=0;i<8;i++)
	{
		RJ45_1_Write_Register(Sn_TXMEM_SIZE(i),W5200_Tx_Size[i]);
		RJ45_1_Write_Register(Sn_RXMEM_SIZE(i),W5200_Rx_Size[i]);
	}
}

//TCPģʽ��ʼ��        ������
u8 RJ45_1_TCP_ServiceInit(void)
{
	u8 i,j=0;
	RJ45_1_Write_Register(SIMR,1<<0); //����SOCKET0�����ж�                  
	RJ45_1_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_TIMEOUT|Sn_IR_DISCON|Sn_IR_CON); //�����ж����μĴ���
	CloseSocket_RJ45_1();
	RJ45_1_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcpģʽ������ʱ
	WriteTem[0]=RJ45_1_Loc_Potr/256;
	WriteTem[1]=RJ45_1_Loc_Potr%256;
	RJ45_1_Write_Buf(Sn_PORT0(0),WriteTem,2);      //���ö˿ں�
  Init1:	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_OPEN);
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	if(RJ45_1_Read_Register(Sn_SR(0))!=SOCK_INIT)   //������ڿ���״̬
	{
		j++;
		if(j<10)
		{
			CloseSocket_RJ45_1();
			goto Init1;
		}
		else
		{
			CloseSocket_RJ45_1();
			return Fail;
		}
	}
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_LISTEN);   //��������״̬
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	return Success;
}
/**
*@brief		��ȡ����
*@param   RData����ȡ���ݴ������
*@return	��
*/
void RJ45_1_Read(u8 *RData)
{
	u16 Start_Address;
	u32 Address;
	RJ45_1_Read_Buf(Sn_RX_RSR0(0),ReadTem,2);
	RJ45_1_RLength =ReadTem[0]*256+ReadTem[1];     //��ȡ���յ����ݳ���
	RJ45_1_Read_Buf(Sn_RX_RD0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];       //��ȡ��Ե�ַ
	WriteTem[0]=(Start_Address+RJ45_1_RLength)/256;
	WriteTem[1]=(Start_Address+RJ45_1_RLength)%256;
	Address=(u32)(Start_Address<<8)+(0<<5)+0x18;   //�õ����Ե�ַ
	RJ45_1_Read_Buf(Address,RData,RJ45_1_RLength); //��ȡ����
	RJ45_1_Write_Buf(Sn_RX_RD0(0),WriteTem,2);      //д���ȡָ��
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_RECV);
	while(RJ45_1_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}
/**
*@brief		��������
*@param   WData����������
*@param   Len��  �������ݳ���,Len<=8K
*@return	��
*/
void RJ45_1_Write(u8 *WData,u16 Len)
{
	u16 ret;
	u16 Start_Address;
	u32 Address;
	RJ45_1_Read_Buf(Sn_TX_WR0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];      //��ȡд���ַָ��
	Address=(u32)(Start_Address<<8)+(0<<5)+0x10;  //������Ե�ַ
	RJ45_1_Write_Buf(Address,WData,Len);          //д�뻺��
	Start_Address+=Len;
	WriteTem[0]=Start_Address/256;
	WriteTem[1]=Start_Address%256;
	RJ45_1_Write_Buf(Sn_TX_WR0(0),WriteTem,2);    //����ָ��
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_SEND);   //����
	while(RJ45_1_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}

/**
*@brief		�ر�����
*@return	��
*/
void CloseSocket_RJ45_1(void)
{
	u8 i;
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_CLOSE);
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	RJ45_1_Write_Register(Sn_IR(0),0xFF);					/*All clear*/
}
/**
*@brief		��W5500д��һ��8λ����
*@param		Address: д�����ݵĵ�ַ
*@param   Data��д�������
*@return	��
*/
void RJ45_1_Write_Register(u32 Address,u8 Data)
{
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_1_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_1_ReadWriteByte((Address & 0x000000F8) + 4) ; 
	RJ45_1_ReadWriteByte(Data);                
	RJ45_1_CS_High ;
}
/**
*@brief		��W5500д��len�ֽ�����
*@param		Address: д�����ݵĵ�ַ
*@param   Buf��д���ֽڵ�ַ
*@param   Len��д���ֽڳ���
*/
void RJ45_1_Write_Buf(u32 Address,u8 *Buf,u16 Len)
{
	u16 i;
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_1_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_1_ReadWriteByte((Address & 0x000000F8) + 4) ; 
	for(i=0;i<Len;i++) 
	{
			RJ45_1_ReadWriteByte(Buf[i]);
	}	
	RJ45_1_CS_High ;
}
/**
*@brief		��W5500����һ��8λ����
*@param		Address: д�����ݵĵ�ַ
*@return Data����д��ĵ�ַ����ȡ����8λ����
*/
u8 RJ45_1_Read_Register(u32 Address)
{
	u8 Data;
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_1_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_1_ReadWriteByte((Address & 0x000000F8)) ; 
	Data=RJ45_1_ReadWriteByte(0x00);            // Data write (write 1byte data)
	RJ45_1_CS_High ;	
	return Data;
}
/**
*@brief		��W5500����Len�ֽ�����
*@param		Address: ��ȡ���ݵĵ�ַ
*@param 	Buf����Ŷ�ȡ����
*@param		len���ֽڳ���
*/
void RJ45_1_Read_Buf(u32 Address,u8 *Buf,u16 Len)
{
	u16 i;
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_1_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_1_ReadWriteByte((Address & 0x000000F8)) ; 
	for(i=0;i<Len;i++)
	{
		Buf[i]=RJ45_1_ReadWriteByte(0x00);
	}
	RJ45_1_CS_High ;	
}
//SPI2���ݶ�д
u8  RJ45_1_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  	
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ��byte  ����
		
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte   
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����	
}

//�ⲿ�жϴ�����
void EXTI15_10_IRQHandler(void)
{
	u16 len;
	u8 StateFlag;
	u8 i;
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		
		StateFlag=RJ45_1_Read_Register(Sn_IR(0));
		if(StateFlag&Sn_IR_CON)
		{
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_CON);
			LED5=!LED5 ;
		}
		if(StateFlag&Sn_IR_RECV)
		{
			RJ45_1_Read(RJ45_1_RData);
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_RECV);
			
			LED4=!LED4 ;
		}
		if(StateFlag&Sn_IR_DISCON)
		{
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_DISCON);
			LED3=!LED3 ;
		}
//		StateFlag =RJ45_1_Read_Register(Sn_IR);
//    if(StateFlag&CON)
//		{		
//			RJ45_1_Write_Register(Sn_IR ,CON);
//			RJ45_1_Connect=1;
//			LED5 =1;
//		}
//		if(StateFlag&DISCON_Flag)
//		{
//			RJ45_1_Write_Register(Sn_IR ,DISCON_Flag);
//			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
//			LED5=0;
//			RJ45_1_Connect=0;
//		}
//		if(StateFlag&RECEV)
//		{	
//			RJ45_1_Read(RJ45_1_RData);
//			RJ45_1_ReceiveFlag=1;
//			RJ45_1_Write_Register(Sn_IR,RECEV);		
//		}
		EXTI_ClearITPendingBit(EXTI_Line14);//���LINE15�ϵ��жϱ�־λ 
	} 
}

/********************RJ45_2����*****************************/
/**
--------------|-------------
        RST  <->  PA2
        PWDN <->  PA3
				CS   <->  PA4
		    CLK  <->  PA5
        MISO <->  PA6
		    MOSI <->  PA7
				INT  <->  PC5
  */
//void RJ45_2_GPIO_Init(void)
//{
//	GPIO_InitTypeDef  GPIO_InitStructure;	
//	EXTI_InitTypeDef  EXTI_InitStructure;
//	NVIC_InitTypeDef  NVIC_InitStructure;
//	SPI_InitTypeDef  SPI_InitStructure;
//	
//	//RST
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//��Ӧ����
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
//	GPIO_SetBits(GPIOA,GPIO_Pin_2);//���ߣ�����λ
//	
//	//PWDN
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//��Ӧ����
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
//	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//���ͣ���������ģʽ
//	
//	//SPI1
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//ʹ��SPI2ʱ��
//	//PA4
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//��Ӧ����
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
//	GPIO_SetBits(GPIOA,GPIO_Pin_4);//���ߣ�ʧ��SPI����
//	
//	//���Ÿ���
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PA5~7���ù������	
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��
//	
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); //PA5����Ϊ SPI1
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1); //PA6����Ϊ SPI1
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); //PA7����Ϊ SPI1
//	
//	SPI_I2S_DeInit(SPI1);
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
//	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
//	
//	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���	
//	SPI_Cmd(SPI1, ENABLE);
//	
//	//INT
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOAʱ��
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//RST��ӦIO��
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);//PE14 ���ӵ��ж���14
//	
//	/* ����EXTI_Line5 */
//	EXTI_InitStructure.EXTI_Line = EXTI_Line5;//LINE5
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ش��� 
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE5
//	EXTI_Init(&EXTI_InitStructure);//����
//	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//�ⲿ�ж�10
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�0
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
//	NVIC_Init(&NVIC_InitStructure);//����	
//}
//	
//void RJ45_2_Reset(void)
//{
//	uint16_t i,j;
//	RJ45_2_RST_Low ;
//	for(i=0;i<165;i++);
//	RJ45_2_RST_High ;
//	for(i=0;i<5000;i++)
//	{
//		for(j=0;j<1000;j++);
//	}
//}

//void RJ45_2_Init(void)
//{
//	u8 i;
//	RJ45_2_GPIO_Init();
//	RJ45_2_Reset();
//	//����MAC ADDRESS
//	for(i=0;i<6;i++)
//	{
//		RJ45_2_Write_Register(SHAR+i,RJ45_2_MAC[i]);
//	}
//	//����Subnet Mask ADDRESS
//	for(i=0;i<4;i++)
//	{	
//		RJ45_2_Write_Register(SUBR+i,RJ45_2_SubNet[i]);
//	}
//	//����GateWay ADDRESS
//	for(i=0;i<4;i++)
//	{
//		RJ45_2_Write_Register(GAR+i,RJ45_2_GateWay[i]);	
//	}
//	//����Local IP ADDRESS
//	for(i=0;i<4;i++)
//	{
//		RJ45_2_Write_Register(SIPR+i,RJ45_2_IP[i]);		
//	}
//	//�������·���ʱ���Լ��ط�����
//	RJ45_2_Write_Register(RTR,(u8)((RTR_Time&0xFF00)>>8));
//	RJ45_2_Write_Register(RTR+1,(u8)(RTR_Time&0x00FF));
//	RJ45_2_Write_Register(RCR,RCR_Num);
//	//���ø�Socket���͡�����������ݰ�
//	for(i=0;i<8;i++)
//	{
//		RJ45_2_Write_Register(Sn_RXMEM_SIZE+i*0x0100,W5200_Rx_Size[i]);
//		RJ45_2_Write_Register(Sn_TXMEM_SIZE+i*0x0100,W5200_Tx_Size[i]);
//	}	
//}

//u8 RJ45_2_TCP_Init(void)
//{
//	u8 i;
//	RJ45_2_Write_Register(W5200_IMR,1<<0);                     //����SOCKET0�����ж�
//	RJ45_2_Write_Register(Sn_IMR ,(u8)(RECEV|DISCON_Flag|CON)); //�����ж����μĴ���
//	Init1:	RJ45_2_Write_Register(Sn_MR ,TCP_MODE);                    //Set TCP mode
//	RJ45_2_Write_Register(Sn_PORT ,(u8)((RJ45_2_Loc_Potr&0xFF00)>>8));  //���ñ����˿ں�   ��λ
//	RJ45_2_Write_Register(Sn_PORT+1 ,(u8)(RJ45_2_Loc_Potr&0x00FF));     //���ñ����˿ں�   ��λ
//	RJ45_2_Write_Register(Sn_CR ,OPEN ); 
//	if(RJ45_2_Read_Register(Sn_SR)!=SOCK_INIT)
//	{
//		i++;
//		if(i<10)
//		{
//			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
//			goto Init1;
//		}
//		else
//		{
//			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
//			return Fail;
//		}
//	}
//	RJ45_2_Write_Register(Sn_CR ,LISTEN);
//	if(RJ45_2_Read_Register(Sn_SR)!=SOCK_LISTEN)
//	{
//		i++;
//		if(i<10)
//		{
//			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
//			goto Init1;
//		}
//		else
//		{
//			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
//			return Fail;
//		}
//	}
//	return Success;
//}

//void RJ45_2_Read(u8 *RData)
//{
//	u16 Len,Start_Address,i,RD_Num,DataLength;
//	RData[0]=RJ45_2_Read_Register(Sn_Rx_RSR);
//	RData[1]=RJ45_2_Read_Register(Sn_Rx_RSR+1);  //��ȡ���յ����ֽڳ���
//	Len=(u16)(RData[0]<<8)+RData[1];
//	Len=Len&Sn_RX_Mask;
//	RJ45_2_DataLength=Len;
//	Start_Address=RJ45_2_Read_Register(Sn_RX_RD);
//	Start_Address=(Start_Address&0x00FF)<<8;	
//	Start_Address+=RJ45_2_Read_Register(Sn_RX_RD+1); //��ȡ��ʼ��ַ
//	RD_Num =Start_Address +Len;
//	Start_Address=Start_Address&Sn_RX_Mask;
//	if(Start_Address+Len>Sn_RX_Mask+1)
//	{
//		DataLength = Sn_RX_Mask+1-Start_Address;
//		Start_Address+=Sn_RX_Base;			
//		RJ45_2_CS_Low ;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
//		RJ45_2_ReadWriteByte(DataLength&0x00FF);
//		for(i=0;i<DataLength;i++)
//		{
//			RData[i]=RJ45_2_ReadWriteByte(0x00);
//		}
//		Start_Address =Sn_RX_Base ;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x00|(((Len-DataLength)&0x7F00)>>8));
//		RJ45_2_ReadWriteByte((Len-DataLength)&0x00FF);
//		for(i=0;i<(Len-DataLength);i++)
//		{
//			RData[i+DataLength]=RJ45_2_ReadWriteByte(0x00);
//		}
//		RJ45_2_CS_High ;	
//	}
//	else
//	{
//		Start_Address+=Sn_RX_Base;
//		DataLength = Len;
//		RJ45_2_CS_Low ;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
//		RJ45_2_ReadWriteByte(DataLength&0x00FF);
//		for(i=0;i<DataLength;i++)
//		{
//			RData[i]=RJ45_2_ReadWriteByte(0x00);
//		}
//		RJ45_2_CS_High ;	
//	}	
//	RJ45_2_Write_Register(Sn_RX_RD ,(RD_Num&0xFF00)>>8); 			  
//	RJ45_2_Write_Register(Sn_RX_RD+1 ,RD_Num&0x00FF);	
//	RJ45_2_Write_Register(Sn_CR ,RECV); 
//	while(RJ45_2_Read_Register(Sn_CR));
//}

//void RJ45_2_Write(u8 *WData,u16 Len)
//{
//	u16 i,Start_Address,DataLength,WR_Num;
//	Start_Address =(RJ45_2_Read_Register(Sn_TX_WR))<<8;
//	Start_Address +=RJ45_2_Read_Register(Sn_TX_WR+1);
//	WR_Num = Start_Address+Len;
//	Start_Address =Start_Address&Sn_TX_Mask;
//	if(Start_Address+Len>Sn_TX_Mask+1)
//	{
//		DataLength =Sn_TX_Mask +1-Start_Address;
//		Start_Address+=Sn_TX_Base;
//		RJ45_2_CS_Low ;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
//		RJ45_2_ReadWriteByte(DataLength&0x00FF);
//		for(i=0;i<DataLength;i++)
//		{
//			RJ45_2_ReadWriteByte(WData[i]);
//		}
//		Start_Address = Sn_TX_Base;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x80|(((Len-DataLength)&0x7F00)>>8));
//		RJ45_2_ReadWriteByte((Len-DataLength)&0x00FF);
//		for(i=0;i<Len-DataLength;i++)
//		{
//			RJ45_2_ReadWriteByte(WData[DataLength+i]);
//		}
//		RJ45_2_CS_High ;
//		RJ45_2_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
//		RJ45_2_Write_Register(Sn_TX_WR+1,(WR_Num&0x00FF));
//	}
//	else
//	{
//		Start_Address =Start_Address &Sn_TX_Mask;
//		DataLength =Len;
//		Start_Address+=Sn_TX_Base;
//		RJ45_2_CS_Low ;
//		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
//		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
//		RJ45_2_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
//		RJ45_2_ReadWriteByte(DataLength&0x00FF);
//		for(i=0;i<DataLength;i++)
//		{
//			RJ45_2_ReadWriteByte(WData[i]);
//		}
//		RJ45_2_CS_High ;
//		RJ45_2_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
//		RJ45_2_Write_Register(Sn_TX_WR+1,WR_Num&0x00FF);
//	}
//	RJ45_2_Write_Register(Sn_CR ,SEND);
//	while(RJ45_2_Read_Register(Sn_CR));
//}

//void RJ45_2_Write_Register(u16 Address,u8 Data)
//{
//	RJ45_2_CS_Low ;
//	RJ45_2_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
//	RJ45_2_ReadWriteByte((Address&0x00FF));     // Address byte 2
//	RJ45_2_ReadWriteByte(0x80);                 // Data write command and Write data length 1
//	RJ45_2_ReadWriteByte(0x01);                 // Write data length 2
//	RJ45_2_ReadWriteByte(Data);                 // Data write (write 1byte data)
//	RJ45_2_CS_High ;	
//}

////��ȡ��ؼĴ�������
//u8 RJ45_2_Read_Register(u16 Address)
//{
//	u8 Data;
//	RJ45_2_CS_Low ;
//	RJ45_2_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
//	RJ45_2_ReadWriteByte((Address&0x00FF));     // Address byte 2
//	RJ45_2_ReadWriteByte(0x00);                 // Data Read command and Write data length 1
//	RJ45_2_ReadWriteByte(0x01);                 // Write data length 2
//	Data=RJ45_2_ReadWriteByte(0x00);                 // Data write (write 1byte data)
//	RJ45_2_CS_High ;
//	return Data;
//}

////SPI1���ݶ�д
//u8  RJ45_2_ReadWriteByte(u8 TxData)
//{
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  	
//	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ��byte  ����
//		
//  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte   
//	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����	
//}


////�ⲿ�жϴ�����
//void EXTI9_5_IRQHandler(void)
//{
////	u16 len;
////	u8 i,EventInformation;
////	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
////	{
////		EventInformation=RJ45_2_Read_Register(Sn_IR);
////    if(EventInformation&CON)
////		{
////		  LED4 =1;
////			RJ45_2_Connect=1;
////			RJ45_2_Write_Register(Sn_IR ,CON);
////		}
////		if(EventInformation&DISCON_Flag)
////		{
////			RJ45_2_Write_Register(Sn_IR ,DISCON_Flag);
////			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
////			LED4 =0;	
////			RJ45_2_Connect=0;
////			RJ45_2_Init();
////			delay_ms(1);
////			RJ45_2_TCP_Init();			
////		}
////		if(EventInformation&RECEV)
////		{	
////			RJ45_2_Read(RJ45_2_RData);
//////			RJ45_2_Deal();
////			ReDevice++;
////			RJ45_2_Write_Register(Sn_IR,RECEV);		
////		}
////		EXTI_ClearITPendingBit(EXTI_Line5);
////	}
//}
//void RJ45_2_Deal(void)     //�������ݴ���
//{
//	u8 i;
//	if((RJ45_2_RData[0]==0x3C)&&(RJ45_2_RData[RJ45_2_DataLength-1]==0x3E))    //�ж����ݰ�ͷ��β
//	{
//		if(RJ45_2_RData[1]==0x00)  //����Խ�������
//		{
//				RJ45_2_WData[0]=0x3C;
//				RJ45_2_WData[1]=0x00;
//			  RJ45_2_WData[2]=0x3E;
//				RJ45_2_Write(RJ45_2_WData,3);
//			  RJ45_2_Connect=1;
//		}
//		if(RJ45_2_RData[1]==0x01)  //����ԶϿ�����
//		{
//				RJ45_2_WData[0]=0x3C;
//				RJ45_2_WData[1]=0x01;
//			  RJ45_2_WData[2]=0x3E;
//				RJ45_2_Write(RJ45_2_WData,3);
//			  RJ45_2_Connect=0;
//		}
//    if(RJ45_2_RData[1]==0x02)  //������Ϣȷ��
//		{
//		}
//		if(RJ45_2_RData[1]==0x03)  //������Ϣȷ��
//		{
//		}
//		if(RJ45_2_RData[1]==0x04)  //ͨ��Ƶ�����
//		{
//			  ToDevice=RJ45_2_RData[2]*256+RJ45_2_RData[3];
//			  Voltage =RJ45_2_RData[5];
//			  DeviceScale =RJ45_2_RData[4];
//			  RJ45_2_WData[0]=0x3C;
//				RJ45_2_WData[1]=0x04;
//			  RJ45_2_WData[2]=0x3E;
//			  RJ45_2_Write(RJ45_2_WData,3);
//		}
//	}
//}
//void Message_Send(void)    //��������Ϣ����������
//{
//	u8 i;
//	RJ45_2_WData[0]=0x3C;
//	RJ45_2_WData[1]=0x02;
//	RJ45_2_WData[2]=ToDevice/256;
//	RJ45_2_WData[3]=ToDevice%256;
//	for(i=0;i<18;i++)
//	{
//		if(ChannelSend&1<<i)
//		{
//			RJ45_2_WData[2*i+4]=i+1;
//			RJ45_2_WData[2*i+5]=ChannelFrenquence[i];
//		}
//		else
//		{
//			RJ45_2_WData[2*i+4]=0;
//			RJ45_2_WData[2*i+5]=0;
//		}
//	}
//	RJ45_2_WData[40]=Voltage;
//	RJ45_2_WData[41]=0x3E;
//	if(RJ45_2_Connect==1)
//	{
//		NVIC_EXTI(0);   //���������ⲿ�ж�
//		RJ45_2_Write(RJ45_2_WData,42);
//		NVIC_EXTI(1);  //���������ⲿ�ж�
//	}
//}
//void Message_Rece(void)    //��������Ϣ����������
//{
//	u8 i;
//	RJ45_2_WData[0]=0x3C;
//	RJ45_2_WData[1]=0x03;
//	ReDevice=0;
//	for(i=0;i<18;i++)
//	{
//		if(ChannelReceive&1<<i)
//		{
//			RJ45_2_WData[2*i+4]=i+1;
//			RJ45_2_WData[2*i+5]=ChannelFrenquence[i];
//			if(ReDevice==0)
//			{
//				ReDevice=SM2200RxBuf[i][42]*256+SM2200RxBuf[i][43];    
//			}
//		}
//		else
//		{
//			RJ45_2_WData[2*i+4]=0;
//			RJ45_2_WData[2*i+5]=0;
//		}
//	}
//	RJ45_2_WData[2]=ReDevice/256;
//	RJ45_2_WData[3]=ReDevice%256;
//	RJ45_2_WData[40]=Voltage;
//	RJ45_2_WData[41]=0x3E;
//	if(RJ45_2_Connect==1)
//	{
//		NVIC_EXTI(0);   //���������ⲿ�ж�
//		RJ45_2_Write(RJ45_2_WData,42);
//		NVIC_EXTI(1);  //���������ⲿ�ж�
//	}
//}


