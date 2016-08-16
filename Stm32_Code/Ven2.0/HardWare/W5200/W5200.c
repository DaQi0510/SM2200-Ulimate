#include "W5200.h"
#include "led.h"
#include "wwdg.h"
#include "sm2200.h"
#include "AT24C02.h"
extern u8  ConnectState;       //�豸����״̬

extern volatile u8 ConnectDevice[7];     //�����豸��
extern volatile u8 DeviceScale;          //���Ӵ�������   0�� 1��

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
extern u8 RJ45_1_DirIP[4];   //�Է�������IP��ַ
extern u16 RJ45_1_Dir_Port;   
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

extern volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
extern volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern u8 Voltage;                   //��¼��ѹ��ֵ
extern volatile u16 ToDevice;        //Ҫ���͵����ݵ��豸��  
extern volatile u8 ReDevice;        //���յ����ݵ��豸��

/******��λ�������*********/
u8 Connect=0x00;      //������������ 
u8 DisConnect=0x01;   //�Ͽ��������� 
u8 PollChat=0x02;     //��ѯͨ����������ַ
u8 RunModes=0x03; //��ȡͨ��������Ϣ
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
	for(i=0;i<50;i++)
	{
		for(j=0;j<2500;j++);
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
	RJ45_1_Write_Register(Sn_KPALVTR(0),1);        //ÿ5s�Զ����һ������״̬
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
*@brief		TCPģʽ��ʼ��  �ͻ���   Ĭ���������Σ��������Ӳ��ϣ�����Fail
*@param   ��
*@return	��
*/
u8 RJ45_1_TCP_ClientInit(void)
{
	u8 i,j=0;
	RJ45_1_Write_Register(SIMR,1<<0); //����SOCKET0�����ж�                  
	RJ45_1_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_DISCON|Sn_IR_CON); //�����ж����μĴ���
	CloseSocket_RJ45_1();
	RJ45_1_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcpģʽ������ʱ
	WriteTem[0]=RJ45_1_Loc_Potr/256;
	WriteTem[1]=RJ45_1_Loc_Potr%256;
	RJ45_1_Write_Buf(Sn_PORT0(0),WriteTem,2);      //���ö˿ں�
  WriteTem[0]=RJ45_1_Dir_Port/256;
	WriteTem[1]=RJ45_1_Dir_Port%256;
	RJ45_1_Write_Buf(Sn_DPORT0(0),WriteTem,2);     //����Ŀ��������˿ں�
	RJ45_1_Write_Buf(Sn_DIPR0(0),RJ45_1_DirIP,4);  //Ŀ�������IP��ַ
	RJ45_1_Write_Register(Sn_KPALVTR(0),1);        //ÿ5s�Զ����һ������״̬
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
	
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_CONNECT);   //��������
	RJ45_1_Read_Buf(Sn_DIPR0(0),RJ45_1_DirIP,4);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{ 
	}
	while(RJ45_1_Read_Register(Sn_SR(0))!=SOCK_SYNSENT)
	{
		if(RJ45_1_Read_Register(Sn_SR(0))==SOCK_ESTABLISHED)
		{
			return Success;
		}
		if(RJ45_1_Read_Register(Sn_IR(0))& Sn_IR_TIMEOUT)
		{
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_TIMEOUT);
			return Fail;
		}
	}
}

/**
*@brief		��ȡ����
*@param   RData����ȡ���ݴ������
*@return	fail 0  success 1
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
	RJ45_1_Write_Register(Sn_IR(0),0xFF);		  /*All clear*/
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
	u8 StateFlag;
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{		
		StateFlag=RJ45_1_Read_Register(Sn_IR(0));
		if(StateFlag&Sn_IR_CON)
		{
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_CON);
		}
		if(StateFlag&Sn_IR_RECV)
		{
			RJ45_1_Read(RJ45_1_RData);
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_RECV);
		}
		if(StateFlag&Sn_IR_DISCON)
		{
			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_DISCON);
			CloseSocket_RJ45_1();
		}
//    if(StateFlag&Sn_IR_TIMEOUT)  //���ӳ�ʱ
//		{
//			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_TIMEOUT);
//			CloseSocket_RJ45_1();
//		}
		EXTI_ClearITPendingBit(EXTI_Line14);//���LINE15�ϵ��жϱ�־λ 
	} 
}

/********************RJ45_2����*****************************/
/**
--------------|-------------
        RST  <->  PA2
				CS   <->  PA4
		    CLK  <->  PA5
        MISO <->  PA6
		    MOSI <->  PA7
				INT  <->  PC5
  */
void RJ45_2_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	//RST
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_2);//���ߣ�����λ
	
	
	//SPI2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOBʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//ʹ��SPI2ʱ��
	
	//PA4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_4);//���ߣ�ʧ��SPI����
	
	//���Ÿ���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PB13~15���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); //PA5����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1); //PA6����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); //PA7����Ϊ SPI1
	
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	
	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���	
	SPI_Cmd(SPI1, ENABLE);
	
	//INT
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//INT��ӦIO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);//PC5 ���ӵ��ж���5
	
	/* ����EXTI_Line14 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;//LINE5
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ش��� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE10
	EXTI_Init(&EXTI_InitStructure);//����
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//�ⲿ�ж�10
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//����	
}

void RJ45_2_Reset(void)
{
	uint16_t i,j;
	RJ45_2_RST_Low ;
	for(i=0;i<25000;i++);
	RJ45_2_RST_High ;
	for(i=0;i<50;i++)
	{
		for(j=0;j<2500;j++);
	}
}

void RJ45_2_Init(void)
{
	u8 i;
	RJ45_2_GPIO_Init();
	RJ45_2_Reset();
	//����MAC ADDRESS
	RJ45_2_Write_Buf(SHAR0,RJ45_2_MAC,6);
	//����Subnet Mask ADDRESS
	RJ45_2_Write_Buf(SUBR0,RJ45_2_SubNet,4);
	//����GateWay ADDRESS
	RJ45_2_Write_Buf(GAR0,RJ45_2_GateWay,4);
	//����Local IP ADDRESS
	RJ45_2_Write_Buf(SIPR0,RJ45_2_IP,4);

	//�������·���ʱ���Լ��ط�����
	WriteTem[0]=RTR_Time/256;
	WriteTem[1]=RTR_Time%256;
	RJ45_2_Write_Buf(RTR0,WriteTem,2);
	RJ45_2_Write_Register(WIZ_RCR,RCR_Num);
	//���ø�Socket���͡�����������ݰ�
	for(i=0;i<8;i++)
	{
		RJ45_2_Write_Register(Sn_TXMEM_SIZE(i),W5200_Tx_Size[i]);
		RJ45_2_Write_Register(Sn_RXMEM_SIZE(i),W5200_Rx_Size[i]);
	}
}

//TCPģʽ��ʼ��        ������
u8 RJ45_2_TCP_ServiceInit(void)
{
	u8 i,j=0;
	RJ45_2_Write_Register(SIMR,1<<0); //����SOCKET0�����ж�                  
	RJ45_2_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_TIMEOUT|Sn_IR_DISCON|Sn_IR_CON); //�����ж����μĴ���
	CloseSocket_RJ45_2();
	RJ45_2_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcpģʽ������ʱ
	WriteTem[0]=RJ45_2_Loc_Potr/256;
	WriteTem[1]=RJ45_2_Loc_Potr%256;
	RJ45_2_Write_Buf(Sn_PORT0(0),WriteTem,2);      //���ö˿ں�
	RJ45_2_Write_Register(Sn_KPALVTR(0),1);        //ÿ5s�Զ����һ������״̬
  Init1:	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_OPEN);
	for(i=0;i<20;i++);
	while(RJ45_2_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	if(RJ45_2_Read_Register(Sn_SR(0))!=SOCK_INIT)   //������ڿ���״̬
	{
		j++;
		if(j<10)
		{
			CloseSocket_RJ45_2();
			goto Init1;
		}
		else
		{
			CloseSocket_RJ45_2();
			return Fail;
		}
	}
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_LISTEN);   //��������״̬
	for(i=0;i<20;i++);
	while(RJ45_2_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
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
void RJ45_2_Read(u8 *RData)
{
	u16 Start_Address;
	u32 Address;
	RJ45_2_Read_Buf(Sn_RX_RSR0(0),ReadTem,2);
	RJ45_2_RLength =ReadTem[0]*256+ReadTem[1];     //��ȡ���յ����ݳ���
	RJ45_2_Read_Buf(Sn_RX_RD0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];       //��ȡ��Ե�ַ
	WriteTem[0]=(Start_Address+RJ45_2_RLength)/256;
	WriteTem[1]=(Start_Address+RJ45_2_RLength)%256;
	Address=(u32)(Start_Address<<8)+(0<<5)+0x18;   //�õ����Ե�ַ
	RJ45_2_Read_Buf(Address,RData,RJ45_2_RLength); //��ȡ����
	RJ45_2_Write_Buf(Sn_RX_RD0(0),WriteTem,2);      //д���ȡָ��
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_RECV);
	while(RJ45_2_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}
/**
*@brief		��������
*@param   WData����������
*@param   Len��  �������ݳ���,Len<=8K
*@return	��
*/
void RJ45_2_Write(u8 *WData,u16 Len)
{
	u16 Start_Address;
	u32 Address;
	RJ45_2_Read_Buf(Sn_TX_WR0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];      //��ȡд���ַָ��
	Address=(u32)(Start_Address<<8)+(0<<5)+0x10;  //������Ե�ַ
	RJ45_2_Write_Buf(Address,WData,Len);          //д�뻺��
	Start_Address+=Len;
	WriteTem[0]=Start_Address/256;
	WriteTem[1]=Start_Address%256;
	RJ45_2_Write_Buf(Sn_TX_WR0(0),WriteTem,2);    //����ָ��
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_SEND);   //����
	while(RJ45_2_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}

/**
*@brief		�ر�����
*@return	��
*/
void CloseSocket_RJ45_2(void)
{
	u8 i;
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_CLOSE);
	for(i=0;i<20;i++);
	while(RJ45_2_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	RJ45_2_Write_Register(Sn_IR(0),0xFF);					/*All clear*/
}
/**
*@brief		��W5500д��һ��8λ����
*@param		Address: д�����ݵĵ�ַ
*@param   Data��д�������
*@return	��
*/
void RJ45_2_Write_Register(u32 Address,u8 Data)
{
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_2_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_2_ReadWriteByte((Address & 0x000000F8) + 4) ; 
	RJ45_2_ReadWriteByte(Data);                
	RJ45_2_CS_High ;
}
/**
*@brief		��W5500д��len�ֽ�����
*@param		Address: д�����ݵĵ�ַ
*@param   Buf��д���ֽڵ�ַ
*@param   Len��д���ֽڳ���
*/
void RJ45_2_Write_Buf(u32 Address,u8 *Buf,u16 Len)
{
	u16 i;
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_2_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_2_ReadWriteByte((Address & 0x000000F8) + 4) ; 
	for(i=0;i<Len;i++) 
	{
			RJ45_2_ReadWriteByte(Buf[i]);
	}	
	RJ45_2_CS_High ;
}
/**
*@brief		��W5500����һ��8λ����
*@param		Address: д�����ݵĵ�ַ
*@return Data����д��ĵ�ַ����ȡ����8λ����
*/
u8 RJ45_2_Read_Register(u32 Address)
{
	u8 Data;
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_2_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_2_ReadWriteByte((Address & 0x000000F8)) ; 
	Data=RJ45_2_ReadWriteByte(0x00);            // Data write (write 1byte data)
	RJ45_2_CS_High ;	
	return Data;
}
/**
*@brief		��W5500����Len�ֽ�����
*@param		Address: ��ȡ���ݵĵ�ַ
*@param 	Buf����Ŷ�ȡ����
*@param		len���ֽڳ���
*/
void RJ45_2_Read_Buf(u32 Address,u8 *Buf,u16 Len)
{
	u16 i;
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address & 0x00FF0000)>>16);  
	RJ45_2_ReadWriteByte((Address & 0x0000FF00)>> 8); 
	RJ45_2_ReadWriteByte((Address & 0x000000F8)) ; 
	for(i=0;i<Len;i++)
	{
		Buf[i]=RJ45_2_ReadWriteByte(0x00);
	}
	RJ45_2_CS_High ;	
}



//SPI1���ݶ�д
u8  RJ45_2_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  	
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ��byte  ����
		
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte   
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����	
}


//�ⲿ�жϴ�����
void EXTI9_5_IRQHandler(void)
{
	u8 StateFlag;
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		StateFlag=RJ45_2_Read_Register(Sn_IR(0));
		if(StateFlag&Sn_IR_CON)
		{
			RJ45_2_Write_Register(Sn_IR(0),Sn_IR_CON);
		}
		if(StateFlag&Sn_IR_RECV)
		{
			RJ45_2_Read(RJ45_2_RData);
			RJ45_2_Write_Register(Sn_IR(0),Sn_IR_RECV);
		  RJ45_2_Deal(); 
		}
		if(StateFlag&Sn_IR_DISCON)
		{
			RJ45_2_Write_Register(Sn_IR(0),Sn_IR_DISCON);
		}
		if(StateFlag&Sn_IR_TIMEOUT)
		{
			RJ45_2_Write_Register(Sn_IR(0),Sn_IR_TIMEOUT);
		}
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
}
void RJ45_2_Deal(void)     //�������ݴ���
{
  u8 i;
	if((RJ45_2_RData[0]==0x3C)&&(RJ45_2_RData[RJ45_2_RLength-1]==0x3E))    //�ж����ݰ�ͷ��β
	{
		if(RJ45_2_RData[1]==Connect)  //����Խ�������
		{
				RJ45_2_WData[0]=0x3C;
				RJ45_2_WData[1]=Connect;
			  RJ45_2_WData[7]=0x3E;
				RJ45_2_Write(RJ45_2_WData,8);
			  RJ45_2_Connect=1;
		}
		if(RJ45_2_RData[1]==DisConnect)  //����ԶϿ�����
		{
				RJ45_2_WData[0]=0x3C;
				RJ45_2_WData[1]=DisConnect;
			  RJ45_2_WData[7]=0x3E;
				RJ45_2_Write(RJ45_2_WData,8);
			  RJ45_2_Connect=0;
		}
		if(RJ45_2_RData[1]==PollChat)  //��ѯͨ�Ų���
		{
			if(RJ45_2_RData[2]==0x00)   //ͨ�����ò���
			{
				/*******��ȡ������Ϣ***********/
				DeviceScale =RJ45_2_RData[3];
				for(i=0;i<7;i++)
				{
					ConnectDevice[i]=RJ45_2_RData[4+i];
				}
				Voltage=RJ45_2_RData[11];
				OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);//��Ϊ�����ɿ�2~3��ͨ��
				/******����������Ϣ**********/
				AT24C02_WriteOneByte(0x13,DeviceScale);   //�豸����ģʽ
				AT24C02_WriteOneByte(0x14,Voltage);   //�豸����ģʽ
				for(i=0;i<7;i++)
				{
					AT24C02_WriteOneByte(0x15+i,ConnectDevice[i]);
				}
				/*****����Ӧ����Ϣ**********/
				RJ45_2_Write(RJ45_2_RData,16);
			}
			if(RJ45_2_RData[2]==0x01)   //ͨ�����ö�ȡ����
			{
				RJ45_2_WData[0]=0x3C;
				RJ45_2_WData[1]=PollChat;
				RJ45_2_WData[2]=0x01;
				RJ45_2_WData[3]=DeviceScale;
				for(i=0;i<7;i++)
				{
					RJ45_2_WData[4+i]=ConnectDevice[i];
				}
				RJ45_2_WData[11]=Voltage;
				RJ45_2_WData[15]=0x3E;
				RJ45_2_Write(RJ45_2_WData,16);
			}
		}
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
	}
}
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


