#include "sm2200.h"
#include "wwdg.h"
#include "delay.h"
#include "led.h"

/*******************SM2200����****************************/
extern volatile u8 SM2200TxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
extern volatile u8 ChannelSize[18];         //��ͨ�����ݳ���
extern volatile u8 ChannelType[18];         //��ͨ������ģʽ
extern volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
extern volatile u8 SM2200ReceiveFalg;       //��������ʱ���ձ��
extern volatile u8 SM2200ReadFlag;          //��ǳ������ڶ�ȡ���� 1���ڶ�ȡ  0��ɶ�ȡ

/*******************�豸����**************************/
extern volatile u8  Device;            //�豸��
extern volatile u16 ToDevice;
extern volatile u8 Command[74];       //������������ 
extern volatile u8 Voltage;           //��¼��ѹ��ֵ



/*************SM2200��ʼ��*****************/
void SM2200_Init(void)
{
  u8 i=0;
	SM2200_GPIO();
	delayms(1);
	SPI_SM2200 ();
	RST_High ;
	delayms(10);
	RST_Low;	
	delayms(10);
	/***************TRANCEIVER_CONFIG˵��********************/
	//��ַ0x56    
	/*     0  ECCEN  �����ݰ�ĩβ��Ӿ������У����鿪��
	       1  SNEN   ��������ʶ��һ������²���ʹ��
	       2  TXREG  ����Regulation�źſ���  д1��ֹ��������Լ�����
	       8~11  ��ַλ�趨
	*/
	OfdmXcvrWrite(TRANCEIVER_CONFIG, 2,0x0105);
//	/***************NODE_ADDR0˵��********************/
//	//��ַ0x60  
////	/*     ����ַλ����        */
	OfdmXcvrWrite(NODE_ADDR0, 2,Device);
////	/***************NODE_ADDR0˵��********************/
////	//��ַ0x68  
////	/*     ���ó�0xFF����ַ����ƥ��       */
	OfdmXcvrWrite(NODE_ADDR_MASK0, 2,0x00FF);
	/***************INTERRUPT_MASK˵��********************/
	//��ַ0x50    
	/*     0  PKTRCVD  �������ʵ���ж�
	       1  RESET    ��λʱ�����ж�
	       2  BIUCHG   BIU Status�仯ʱ�����ж�
	       3  TXBUF Empty  ���ͼĴ�����ʱ�����ж�
	*/
	OfdmXcvrWrite(INTERRUPT_MASK,2,0x0001);
	/***************TX_OUT_VOLTAGE˵��********************/
	//��ַ0x3C    
	/* 0~9λ���������ѹ��Χ
	   Voltage=0.03*�Ĵ�����ֵ     ��TRANCEIVER_CONFIG��λ����ʱ���˼Ĵ����ɲ�����
		 SM2200Ĭ����������Χ1.53V
	*/
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);//��Ϊ�����ɿ�2~3��ͨ��
	for(i=0;i<NUMBER_OF_CLUSTERS ;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
		/***************CARRIER_VOLTAGE˵��********************/
		//��ַ0x32    
		/* ÿ��ͨ�����ź�˥��  0~7��Ӧ -21dB~0dB
		*/
		OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);
		/***************CARRIER_FREQ_SELECT˵��********************/
		//��ַ0x22    
		/* �ز�Ƶ��ѡ��
		   ȡֵ��Χ0~101    Ƶ�ʷ�Χ4.88k~498k			
		*/
		OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,ChannelFrenquence[i]);
		/***************TX_BUFFER_CTRL˵��********************/
		//��ַ0x3A   
		/* ���ͼĴ���->����Ƶ��ӳ������	
		*/
		OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	}
	/***************AGC_CONTROL˵��********************/
	//��ַ0x70   
	/* 0~3  �͵�ѹ��ֵ     4~7  �ߵ�ѹ��ֵ  ��������㹫ʽ�������
		 8  AGC0����   9  AGC1����   10   TX_LED����      11 AGCMODE   0->PWMģʽ   1->AGCģʽ
		 12~15  ATTACK   AGC Attack time=(ATTACK+1)*100us
	*/
	OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
	/***************AGC_LEVELS˵��********************/
	//��ַ0x72   
	/* ����ÿ�ε�ѹ���������ֵʱ��AGC���ű仯���	
	*/
	OfdmXcvrWrite(CLUSTER_SELECT,2, 0);
	OfdmXcvrWrite(AGC_LEVELS,2,0x00);
	OfdmXcvrWrite(CLUSTER_SELECT,2,1);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
	OfdmXcvrWrite(CLUSTER_SELECT,2,2);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
	OfdmXcvrWrite(CLUSTER_SELECT,2,3);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
	/***************CHANNEL_ENABLE˵��********************/
	//��ַ0x2A   
	/* ͨ��ʹ�ܼĴ���
	*/
	OfdmXcvrWrite(CHANNEL_ENABLE,3,0x03ffff);  //�������еĽ���ͨ��
	/***************TRANCEIVER_CONTROL********************/
	//��ַ0x24   
	/* �������ƼĴ���
		 0 �����ʹ�ܣ���������ʹ��  1~2  ģʽѡ��  0 0����ģʽ  0 1��ģʽ   1 0 ͨ���Լ��˷����շ�   1 1 �ڲ����շ�
		 3 OPPD �ڲ��˷ſ���  0�����ڲ��˷�   4  ��������   7  CPMODE  ��������   8  SSFRAME ��ֹ����ʧȥͬ�� ��1   
		 9  AGCEN   AGC����ʹ��   10 BIUQUAL û����ϸ˵��  	 
	*/
	OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0310);
}
void SetSm2200Frenquence(u8 ChannelN)
{
	u8 i;
	for(i=0;i<ChannelN ;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
		OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);
		OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,ChannelFrenquence[i]);
		OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	}
}
/*************SM2200�շ�*****************/
//���ͺ���
void SM2200_Send(void)
{
	u8 i,Channel;
	NVIC_EXTI(0);
	for(Channel =0;Channel <18;Channel ++)
	{
		if(ChannelSend &(1<<Channel))
		{
			OfdmXcvrWrite(CLUSTER_SELECT, 2,Channel);
			OfdmXcvrWrite(SPI_PKTSIZE,2,(ChannelType[Channel]<<8)|(ChannelSize[Channel]));
			SPI_NSS_Low;
			SPI3_ReadWriteByte(TRANSMIT_BUFFER|0x80);
			for(i=0;i<ChannelSize[Channel];i++)
			{
				SPI3_ReadWriteByte(SM2200TxBuf[Channel][i]);
			}
			SPI_NSS_High;
		}
	}
	OfdmXcvrWrite(SPI_SEND_PKTS,3,ChannelSend);
	NVIC_EXTI(1);
	ChannelSend=0;
}
//���պ������ж���ִ��
void SM2200_Receive(void)
{
	
	u8 num,Packet_Size,Channel;
	unsigned long Reveive_Channel,Data_Size;
	/***************RECEIVE_STATUS˵��********************/
	//��ַ0x40    
	/*    
		0~18bit ָʾ�ĸ�����ͨ��������
	*/
	Reveive_Channel=OfdmXcvrRead(RECEIVE_STATUS, 3);	
	for(Channel=0;Channel<18;Channel++)
	{
		if((Reveive_Channel&(1<<Channel)))
		{
			ChannelReceive|=1<<Channel;
			OfdmXcvrWrite(CLUSTER_SELECT,2,Channel);
			/***************SPI_PKTSIZE˵��********************/
			//��ַ0x26    
			/*    
				0~18bit ָʾ�ĸ�����ͨ��������
			*/
			Data_Size=OfdmXcvrRead(SPI_PKTSIZE, 2);
			if(Data_Size&0x0100)
			{
				Packet_Size=(u8)(Data_Size&0xff);
				SPI_NSS_Low ;
				SPI3_ReadWriteByte(RECEIVE_BUFFER);				
				for(num=0;num<Packet_Size;num++)
				{
					SM2200RxBuf[Channel][num]=SPI3_ReadWriteByte(0xAA);
				}
				SPI_NSS_High ;	
			}
		}		
	}
	SM2200ReadFlag=0;
}
/*************��Ӧ����������Ϣ********************/
void SM2200Respond(void)
{
	u8 i;
	u16 Amplitude;
	u32 Enable;
	SM2200_GPIO();
	delayms(1);
	SPI_SM2200 ();
	RST_High ;
	delayms(10);
	RST_Low;	
	delayms(10);
	OfdmXcvrWrite(TRANCEIVER_CONFIG, 2,0x0005);
	OfdmXcvrWrite(INTERRUPT_MASK,2,0x0009);
	Amplitude =Command[72]*256+Command[73];
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2, Amplitude);//��Ϊ�����ɿ�2~3��ͨ��
	for(i=0;i<NUMBER_OF_CLUSTERS ;i++)
	{
		if(Command[4*i]==1)
		{
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
			/***************CARRIER_VOLTAGE˵��********************/
			//��ַ0x32    
			/* ÿ��ͨ�����ź�˥��  0~7��Ӧ -21dB~0dB
			*/
			OfdmXcvrWrite(CARRIER_VOLTAGE,2,Command[4*i+3]);
			/***************CARRIER_FREQ_SELECT˵��********************/
			//��ַ0x22    
			/* �ز�Ƶ��ѡ��
				 ȡֵ��Χ0~101    Ƶ�ʷ�Χ4.88k~498k			
			*/
			OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,Command[4*i+1]);
			/***************TX_BUFFER_CTRL˵��********************/
			//��ַ0x3A   
			/* ���ͼĴ���->����Ƶ��ӳ������	
			*/
			OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
			Enable+=1<<i;
			ChannelSize[i]=50;          //����50��Byte��������
			ChannelType [i]=Command [4*i+2];
//			for(j=0;j<50;j++)
//			{
//				SM2200TxBuf[i][j]=Check[j];
//			}
		}
	}
	/***************AGC_CONTROL˵��********************/
	//��ַ0x70   
	/* 0~3  �͵�ѹ��ֵ     4~7  �ߵ�ѹ��ֵ  ��������㹫ʽ�������
		 8  AGC0����   9  AGC1����   10   TX_LED����      11 AGCMODE   0->PWMģʽ   1->AGCģʽ
		 12~15  ATTACK   AGC Attack time=(ATTACK+1)*100us
	*/
	OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
	/***************AGC_LEVELS˵��********************/
	//��ַ0x72   
	/* ����ÿ�ε�ѹ���������ֵʱ��AGC���ű仯���	
	*/
	OfdmXcvrWrite(CLUSTER_SELECT,2, 0);
	OfdmXcvrWrite(AGC_LEVELS,2,0x00);
	OfdmXcvrWrite(CLUSTER_SELECT,2,1);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
	OfdmXcvrWrite(CLUSTER_SELECT,2,2);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
	OfdmXcvrWrite(CLUSTER_SELECT,2,3);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
	/***************CHANNEL_ENABLE˵��********************/
	//��ַ0x2A   
	/* ͨ��ʹ�ܼĴ���
	*/
	OfdmXcvrWrite(CHANNEL_ENABLE,3,Enable);  //�������еĽ���ͨ��
	ChannelSend=Enable;
	Enable=0;
	/***************TRANCEIVER_CONTROL********************/
	//��ַ0x24   
	/* �������ƼĴ���
		 0 �����ʹ�ܣ���������ʹ��  1~2  ģʽѡ��  0 0����ģʽ  0 1��ģʽ   1 0 ͨ���Լ��˷����շ�   1 1 �ڲ����շ�
		 3 OPPD �ڲ��˷ſ���  0�����ڲ��˷�   4  ��������   7  CPMODE  ��������   8  SSFRAME ��ֹ����ʧȥͬ�� ��1   
		 9  AGCEN   AGC����ʹ��   10 BIUQUAL û����ϸ˵��  	 
	*/
	OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0310);
}
/*************SM2200���ų�ʼ��********************/
//��ʼ��SPI_NSS��RST��INT���Ų�����INT�����ⲿ�ж�
void SM2200_GPIO(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	NVIC_InitTypeDef  NVIC_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;

  //SPI_NSS����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//ʹ��GPIODʱ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//SPI_NSS��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOD,GPIO_Pin_0);//Ƭѡ�øߣ���ֹ��д����
	
	//RST����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//RST��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_1);//��λ�øߣ���λSM2200
	
	//INT����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//RST��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	
  //INT�ж�����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);//PA0 ���ӵ��ж���0
	/* ����EXTI_Line15 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;//LINE12
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE12
  EXTI_Init(&EXTI_InitStructure);//����
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//�ⲿ�ж�12
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����	
}
//�ⲿ�жϴ�����
void EXTI0_IRQHandler(void)
{
	u16 Event;
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		Event=OfdmXcvrRead (INTERRUPT_EVENT,2);
		if(Event&0x01)
		{
			SM2200ReadFlag=1;
			SM2200_Receive();
			SM2200ReceiveFalg=1;
		}
		EXTI_ClearITPendingBit(EXTI_Line0);//���LINE0�ϵ��жϱ�־λ 
	}
	 
}

/*************SM2200��д�Ĵ���*****************/
//������
unsigned long OfdmXcvrRead(unsigned char address, unsigned char size)
{
	unsigned char spiByteRead;
  unsigned long returnSpiRead = 0;
  SPI_NSS_Low  ;
	address = address&0x7F;
	spiByteRead=SPI3_ReadWriteByte(address);
	if(size==3)
	{
		spiByteRead=SPI3_ReadWriteByte(address);
		returnSpiRead+=spiByteRead;
		returnSpiRead<<=8;
	}
	spiByteRead=SPI3_ReadWriteByte(address);
	returnSpiRead+=spiByteRead;
	returnSpiRead<<=8;
	spiByteRead=SPI3_ReadWriteByte(address);
	returnSpiRead+=spiByteRead;
	SPI_NSS_High ;
	return returnSpiRead;
}
//д����
void OfdmXcvrWrite(unsigned char address, unsigned char size, unsigned long writeData)
{
	u8 spiByteRead;
	u8 writeCharCast = 0;
	SPI_NSS_Low  ;
	address = (address&0x7F)|0x80;
	spiByteRead=SPI3_ReadWriteByte(address);
	if(size==3)
	{
		writeCharCast=writeData/65536;
		spiByteRead=SPI3_ReadWriteByte(writeCharCast);
	}
	writeCharCast=(writeData%65536)/256;
	spiByteRead=SPI3_ReadWriteByte(writeCharCast);
	writeCharCast=writeData%256;
	spiByteRead=SPI3_ReadWriteByte(writeCharCast);
	SPI_NSS_High;
}

/*************SPI���ֳ�ʼ��********************/
//  SM2200  SPI���ܳ�ʼ��
void SPI_SM2200(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);//ʹ��SPI3ʱ��
 
  //GPIOA5,6,7��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC10~12���ù������	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3); //PA5����Ϊ SPI1
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3); //PA6����Ϊ SPI1
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3); //PA7����Ϊ SPI1
	
  SPI_I2S_DeInit(SPI3);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	
	SPI_Init(SPI3, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���	
	SPI_CalculateCRC(SPI3,DISABLE);
	SPI_Cmd(SPI3, ENABLE);
}

//   SPI���͡��������ݺ���
u8 SPI3_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  	
	SPI_I2S_SendData(SPI3, TxData); //ͨ������SPIx����һ��byte  ����
		
  while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte   
	return SPI_I2S_ReceiveData(SPI3); //����ͨ��SPIx������յ�����	
}
//���������ⲿ�ж�
void NVIC_EXTI(u8 En)
{
	/*****************
	Line0->SM2200 
	Line5->W5200_2  
	Line14->W5200_1  
	*****************/
	if(En)EXTI->IMR|=1<<0 | 1<<5 | 1<<14;   //�������ж�
  else EXTI->IMR&=~(1<<0 | 1<<5 | 1<<14); //�����ж�
}
/**
*@brief		�����ʱ
*@param		num ���Կ�����ʱ��С
*/
void Dealyx(u16 num)
{
	u16 i,j;
	for(j=0;j<num;j++)
	{
		for(i=0;i<20;i++);
	}
}



