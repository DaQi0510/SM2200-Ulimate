#include "sm2200.h"
#include "wwdg.h"
#include "delay.h"

/*******************SM2200部分****************************/
extern volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 ChannelSize[18];         //各通道数据长度
extern volatile u8 ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记

/*******************设备部分**************************/
extern volatile u8  Device;            //设备号
extern volatile u16 ToDevice;
extern volatile u8 Command[74];       //主机控制命令 
extern volatile u8 Voltage;           //记录电压幅值



/*************SM2200初始化*****************/
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
	/***************TRANCEIVER_CONFIG说明********************/
	//地址0x56    
	/*     0  ECCEN  在数据包末尾添加纠错序列，建议开启
	       1  SNEN   数据冗余识别，一般情况下不用使用
	       2  TXREG  允许Regulation信号控制  写1禁止，需程序自己处理
	       8~11  地址位设定
	*/
	OfdmXcvrWrite(TRANCEIVER_CONFIG, 2,0x0205);
//	/***************NODE_ADDR0说明********************/
//	//地址0x60  
////	/*     设别地址位设置        */
	OfdmXcvrWrite(NODE_ADDR0, 2,Device);
////	/***************NODE_ADDR0说明********************/
////	//地址0x68  
////	/*     设置成0xFF，地址必须匹配       */
	OfdmXcvrWrite(NODE_ADDR_MASK0, 2,0xFFFF);
	/***************INTERRUPT_MASK说明********************/
	//地址0x50    
	/*     0  PKTRCVD  接收完成实现中断
	       1  RESET    复位时进入中断
	       2  BIUCHG   BIU Status变化时进入中断
	       3  TXBUF Empty  发送寄存器空时进入中断
	*/
	OfdmXcvrWrite(INTERRUPT_MASK,2,0x0001);
	/***************TX_OUT_VOLTAGE说明********************/
	//地址0x3C    
	/* 0~9位控制输出电压范围
	   Voltage=0.03*寄存器的值     当TRANCEIVER_CONFIG的位置零时，此寄存器可不考虑
		 SM2200默认最大输出范围1.53V
	*/
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);//以为着最多可开2~3个通道
	for(i=0;i<NUMBER_OF_CLUSTERS ;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
		/***************CARRIER_VOLTAGE说明********************/
		//地址0x32    
		/* 每个通道的信号衰减  0~7对应 -21dB~0dB
		*/
		OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);
		/***************CARRIER_FREQ_SELECT说明********************/
		//地址0x22    
		/* 载波频率选择
		   取值范围0~101    频率范围4.88k~498k			
		*/
		OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,ChannelFrenquence[i]);
		/***************TX_BUFFER_CTRL说明********************/
		//地址0x3A   
		/* 发送寄存器->发送频道映射设置	
		*/
		OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	}
	/***************AGC_CONTROL说明********************/
	//地址0x70   
	/* 0~3  低电压限值     4~7  高电压限值  （具体计算公式不清楚）
		 8  AGC0引脚   9  AGC1引脚   10   TX_LED引脚      11 AGCMODE   0->PWM模式   1->AGC模式
		 12~15  ATTACK   AGC Attack time=(ATTACK+1)*100us
	*/
	OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
	/***************AGC_LEVELS说明********************/
	//地址0x72   
	/* 控制每次电压超过最高限值时，AGC引脚变化情况	
	*/
	OfdmXcvrWrite(CLUSTER_SELECT,2, 0);
	OfdmXcvrWrite(AGC_LEVELS,2,0x00);
	OfdmXcvrWrite(CLUSTER_SELECT,2,1);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
	OfdmXcvrWrite(CLUSTER_SELECT,2,2);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
	OfdmXcvrWrite(CLUSTER_SELECT,2,3);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
	/***************CHANNEL_ENABLE说明********************/
	//地址0x2A   
	/* 通道使能寄存器
	*/
	OfdmXcvrWrite(CHANNEL_ENABLE,3,0x03ffff);  //开启所有的接收通道
	/***************TRANCEIVER_CONTROL********************/
	//地址0x24   
	/* 物理层控制寄存器
		 0 物理层使能，必须清零使能  1~2  模式选择  0 0正常模式  0 1空模式   1 0 通过自己运放自收发   1 1 内部自收发
		 3 OPPD 内部运放开关  0开启内部运放   4  接收允许   7  CPMODE  建议置零   8  SSFRAME 防止主从失去同步 置1   
		 9  AGCEN   AGC工能使能   10 BIUQUAL 没有详细说明  	 
	*/
	OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0310);
}
void SetSm2200Frenquence(u8 ChannelN)
{
	u8 i;
	for(i=0;i<ChannelN ;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
		OfdmXcvrWrite(CARRIER_VOLTAGE,2,0x07);
		OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,ChannelFrenquence[i]);
		OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
	}
}
/*************SM2200收发*****************/
//发送函数
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
//接收函数，中断内执行
void SM2200_Receive(void)
{
	
	u8 num,Packet_Size,Channel;
	unsigned long Reveive_Channel,Data_Size;
	/***************RECEIVE_STATUS说明********************/
	//地址0x40    
	/*    
		0~18bit 指示哪个接收通道有数据
	*/
	Reveive_Channel=OfdmXcvrRead(RECEIVE_STATUS, 3);	
	for(Channel=0;Channel<18;Channel++)
	{
		if((Reveive_Channel&(1<<Channel)))
		{
			ChannelReceive|=1<<Channel;
			OfdmXcvrWrite(CLUSTER_SELECT,2,Channel);
			/***************SPI_PKTSIZE说明********************/
			//地址0x26    
			/*    
				0~18bit 指示哪个接收通道有数据
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
}
/*************相应网络配置信息********************/
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
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2, Amplitude);//以为着最多可开2~3个通道
	for(i=0;i<NUMBER_OF_CLUSTERS ;i++)
	{
		if(Command[4*i]==1)
		{
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
			/***************CARRIER_VOLTAGE说明********************/
			//地址0x32    
			/* 每个通道的信号衰减  0~7对应 -21dB~0dB
			*/
			OfdmXcvrWrite(CARRIER_VOLTAGE,2,Command[4*i+3]);
			/***************CARRIER_FREQ_SELECT说明********************/
			//地址0x22    
			/* 载波频率选择
				 取值范围0~101    频率范围4.88k~498k			
			*/
			OfdmXcvrWrite(CARRIER_FREQ_SELECT,2,Command[4*i+1]);
			/***************TX_BUFFER_CTRL说明********************/
			//地址0x3A   
			/* 发送寄存器->发送频道映射设置	
			*/
			OfdmXcvrWrite(TX_BUFFER_CTRL,2,i);
			Enable+=1<<i;
			ChannelSize[i]=50;          //发送50个Byte测试数组
			ChannelType [i]=Command [4*i+2];
//			for(j=0;j<50;j++)
//			{
//				SM2200TxBuf[i][j]=Check[j];
//			}
		}
	}
	/***************AGC_CONTROL说明********************/
	//地址0x70   
	/* 0~3  低电压限值     4~7  高电压限值  （具体计算公式不清楚）
		 8  AGC0引脚   9  AGC1引脚   10   TX_LED引脚      11 AGCMODE   0->PWM模式   1->AGC模式
		 12~15  ATTACK   AGC Attack time=(ATTACK+1)*100us
	*/
	OfdmXcvrWrite(AGC_CONTROL,2,0xABA1);
	/***************AGC_LEVELS说明********************/
	//地址0x72   
	/* 控制每次电压超过最高限值时，AGC引脚变化情况	
	*/
	OfdmXcvrWrite(CLUSTER_SELECT,2, 0);
	OfdmXcvrWrite(AGC_LEVELS,2,0x00);
	OfdmXcvrWrite(CLUSTER_SELECT,2,1);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0010);
	OfdmXcvrWrite(CLUSTER_SELECT,2,2);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0020);
	OfdmXcvrWrite(CLUSTER_SELECT,2,3);
	OfdmXcvrWrite(AGC_LEVELS,2,0x0030);
	/***************CHANNEL_ENABLE说明********************/
	//地址0x2A   
	/* 通道使能寄存器
	*/
	OfdmXcvrWrite(CHANNEL_ENABLE,3,Enable);  //开启所有的接收通道
	ChannelSend=Enable;
	Enable=0;
	/***************TRANCEIVER_CONTROL********************/
	//地址0x24   
	/* 物理层控制寄存器
		 0 物理层使能，必须清零使能  1~2  模式选择  0 0正常模式  0 1空模式   1 0 通过自己运放自收发   1 1 内部自收发
		 3 OPPD 内部运放开关  0开启内部运放   4  接收允许   7  CPMODE  建议置零   8  SSFRAME 防止主从失去同步 置1   
		 9  AGCEN   AGC工能使能   10 BIUQUAL 没有详细说明  	 
	*/
	OfdmXcvrWrite(TRANCEIVER_CONTROL, 2,0x0310);
}
/*************SM2200引脚初始化********************/
//初始化SPI_NSS、RST、INT引脚并设置INT引脚外部中断
void SM2200_GPIO(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	NVIC_InitTypeDef  NVIC_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;

  //SPI_NSS设置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOD时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//SPI_NSS对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOD,GPIO_Pin_0);//片选置高，禁止读写数据
	
	//RST设置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//RST对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_1);//复位置高，复位SM2200
	
	//INT设置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//RST对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
  //INT中断配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);//PA0 连接到中断线0
	/* 配置EXTI_Line15 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;//LINE12
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE12
  EXTI_Init(&EXTI_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//外部中断12
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置	
}
//外部中断处理函数
void EXTI0_IRQHandler(void)
{
	u16 Event;
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		Event=OfdmXcvrRead (INTERRUPT_EVENT,2);
		if(Event&0x01)
		{
			SM2200_Receive();
			SM2200ReceiveFalg=1;
		}
		EXTI_ClearITPendingBit(EXTI_Line0);//清除LINE0上的中断标志位 
	}
	 
}

/*************SM2200读写寄存器*****************/
//读函数
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
//写函数
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

/*************SPI部分初始化********************/
//  SM2200  SPI功能初始化
void SPI_SM2200(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);//使能SPI3时钟
 
  //GPIOA5,6,7初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC10~12复用功能输出	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3); //PA5复用为 SPI1
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3); //PA6复用为 SPI1
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3); //PA7复用为 SPI1
	
  SPI_I2S_DeInit(SPI3);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	
	SPI_Init(SPI3, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器	
	SPI_CalculateCRC(SPI3,DISABLE);
	SPI_Cmd(SPI3, ENABLE);
}

//   SPI发送、接收数据函数
u8 SPI3_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  	
	SPI_I2S_SendData(SPI3, TxData); //通过外设SPIx发送一个byte  数据
		
  while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte   
	return SPI_I2S_ReceiveData(SPI3); //返回通过SPIx最近接收的数据	
}
//屏蔽所有外部中断
void NVIC_EXTI(u8 En)
{
	/*****************
	Line0->SM2200 
	Line5->W5200_2  
	Line14->W5200_1  
	*****************/
	if(En)EXTI->IMR|=1<<0 | 1<<5 | 1<<14;   //不屏蔽中断
  else EXTI->IMR&=~(1<<0 | 1<<5 | 1<<14); //屏蔽中断
}



