#include "W5200.h"
#include "led.h"

extern u8  ConnectState;       //设备连接状态

extern volatile u8 ConnectDevice;     //连接设备号
extern volatile u8 DeviceScale;       //连接从属级别   0从 1主

extern volatile u8 Command[74];       //主机控制命令
extern u8 ServiceIP[4];
extern u16 ServicePort;
extern u8 RJ45_1_MAC[6];
extern u8 RJ45_1_IP[4];
extern u8 RJ45_1_GateWay[4];
extern u8 RJ45_1_SubNet[4];
extern u16 RJ45_1_Loc_Potr;
extern u8 RJ45_1_RData[1024];
extern u8 RJ45_1_WData[1024];
extern u8 RJ45_1_Connect;  //连接状态
extern u8 RJ45_1_ReceiveFlag;
extern u8 RJ45_1_Send;     //发送状态
extern u8 RJ45_2_MAC[6];
extern u8 RJ45_2_IP[4];
extern u8 RJ45_2_GateWay[4];
extern u8 RJ45_2_SubNet[4];
extern u16 RJ45_2_Loc_Potr;
extern u8 RJ45_2_RData[1024];
extern u8 RJ45_2_WData[1024];
extern u8 RJ45_2_Connect;  //连接状态
extern u8 RJ45_2_ReceiveFlag;//标记网口2是否接收到数据
extern u8 RJ45_2_Send;     //发送状态
extern u8 Dest_IP[4] ;
extern u16 RTR_Time; //重新发送时间
extern u8 RCR_Num;   //重新发送次数 
u8 W5200_Rx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Rx_Size[i])K And SUM=16K
u8 W5200_Tx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Tx_Size[i])K And SUM=16K
extern volatile u8 CommandFlag;
u8 EstablishConnect[8]={'C','o','n','n','e','c','t','d'};
u8 DisConnect[8]={'S','h','u','t','d','o','w','n'};

extern u8 ShakeHands[7];
extern u8 Flag;
extern u8 Len;
extern volatile u8 Netflag;   //用于指示当前网络所处状态
extern volatile u8 BER_Flag;

/********************RJ45_1部分*****************************/
/**
--------------|-------------
        RST  <->  PE12
        PWDN <->  PE13
				CS   <->  PB12
		    CLK  <->  PB13
		    MISO <->  PB14
		    MOSI <->  PB15
				INT  <->  PE14
  */
void RJ45_1_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	//RST
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOE,GPIO_Pin_12);//拉高，不复位
	
	//PWDN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO	
	GPIO_ResetBits(GPIOE,GPIO_Pin_13);//拉低，正常工作模式
	
	//SPI2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI2时钟
	
	//PB12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);//拉高，失能SPI传输
	
	//引脚复用
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PB13~15复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); //PB13复用为 SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); //PB14复用为 SPI2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); //PB15复用为 SPI2
	
	SPI_I2S_DeInit(SPI2);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器	
	SPI_Cmd(SPI2, ENABLE);
	
	//INT
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;//INT对应IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource14);//PE14 连接到中断线14
	
	/* 配置EXTI_Line14 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;//LINE10
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE10
	EXTI_Init(&EXTI_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//外部中断10
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置	
}

void RJ45_1_Reset(void)
{
	uint16_t i,j;
	RJ45_1_RST_Low ;
	for(i=0;i<165;i++);
	RJ45_1_RST_High ;
	for(i=0;i<5000;i++)
	{
		for(j=0;j<1000;j++);
	}
}

void RJ45_1_Init(void)
{
	u8 i;
	RJ45_1_GPIO_Init();
	RJ45_1_Reset();
	//设置MAC ADDRESS
	for(i=0;i<6;i++)
	{
		RJ45_1_Write_Register(SHAR+i,RJ45_1_MAC[i]);
	}
	//设置GateWay ADDRESS
	for(i=0;i<4;i++)
	{
		RJ45_1_Write_Register(GAR+i,RJ45_1_GateWay[i]);	
	}
	//设置Local IP ADDRESS
	for(i=0;i<4;i++)
	{

		RJ45_1_Write_Register(SIPR+i,RJ45_1_IP[i]);		
	}
	//设置重新发送时间以及重发次数
	RJ45_1_Write_Register(RTR,(u8)((RTR_Time&0xFF00)>>8));
	RJ45_1_Write_Register(RTR+1,(u8)(RTR_Time&0x00FF));
	RJ45_1_Write_Register(RCR,RCR_Num);
	//设置各Socket发送、接收最大数据包
	for(i=0;i<8;i++)
	{
		RJ45_1_Write_Register(Sn_RXMEM_SIZE+i*0x0100,W5200_Rx_Size[i]);
		RJ45_1_Write_Register(Sn_TXMEM_SIZE+i*0x0100,W5200_Tx_Size[i]);
	}	
}

//TCP模式初始化        客户端
u8 RJ45_1_TCP_ClientInit(void)
{
	u8 i;
	RJ45_1_Write_Register(W5200_IMR,1<<0);                     //允许SOCKET0产生中断
	RJ45_1_Write_Register(Sn_IMR ,(u8)(RECEV|DISCON_Flag|CON)); //设置中断屏蔽寄存器
	Init1:	RJ45_1_Write_Register(Sn_MR ,TCP_MODE);                    //Set TCP mode
	RJ45_1_Write_Register(Sn_PORT ,(u8)((RJ45_1_Loc_Potr&0xFF00)>>8));  //设置本机端口号   高位
	RJ45_1_Write_Register(Sn_PORT+1 ,(u8)(RJ45_1_Loc_Potr&0x00FF));     //设置本机端口号   低位
	RJ45_1_Write_Register(Sn_CR ,OPEN ); 
	if(RJ45_1_Read_Register(Sn_SR)!=SOCK_INIT)
	{
		i++;
		if(i<10)
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			goto Init1;
		}
		else
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			return Fail;
		}
	}
	for(i=0;i<4;i++)
	{
		RJ45_1_Write_Register(Sn_DIPR+i,ServiceIP[i]);
	}
	RJ45_1_Write_Register(Sn_DPORT ,(u8)((ServicePort &0xFF00)>>8));  //设置本机端口号   高位
	RJ45_1_Write_Register(Sn_DPORT+1 ,(u8)(ServicePort&0x00FF)); 
	RJ45_1_Write_Register(Sn_CR ,CONNECT);
	while(RJ45_1_Read_Register(Sn_CR));
	ConnectState =0;
	while(RJ45_1_Read_Register(Sn_SR)!= SOCK_SYNSENT)
	{
		if(RJ45_1_Read_Register(Sn_IR)& TIMEOUT)
		{
			RJ45_1_Write_Register(Sn_IR ,TIMEOUT);
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			ConnectState =1;
			break;
		}
		if(RJ45_1_Read_Register(Sn_SR)== SOCK_ESTABLISHED)
		{
			ConnectState=2;
			break;
		}
	}
	return ConnectState;
}
void RJ45_1_TcpOpen(void)
{
	u8 i;
	RJ45_1_Write_Register(Sn_MR ,TCP_MODE);                    //Set TCP mode
	RJ45_1_Write_Register(Sn_PORT ,(u8)((RJ45_1_Loc_Potr&0xFF00)>>8));  //设置本机端口号   高位
	RJ45_1_Write_Register(Sn_PORT+1 ,(u8)(RJ45_1_Loc_Potr&0x00FF));     //设置本机端口号   低位
	RJ45_1_Write_Register(Sn_CR ,OPEN ); 
	for(i=0;i<100;i++);
	RJ45_1_Write_Register(Sn_CR ,LISTEN);	
}
//TCP模式初始化        服务器
u8 RJ45_1_TCP_ServiceInit(void)
{
	u8 i;
	RJ45_1_Write_Register(W5200_IMR,1<<0);                     //允许SOCKET0产生中断
	RJ45_1_Write_Register(Sn_IMR ,(u8)(RECEV|DISCON_Flag|CON)); //设置中断屏蔽寄存器
	Init1:	RJ45_1_Write_Register(Sn_MR ,TCP_MODE);                    //Set TCP mode
	RJ45_1_Write_Register(Sn_PORT ,(u8)((RJ45_1_Loc_Potr&0xFF00)>>8));  //设置本机端口号   高位
	RJ45_1_Write_Register(Sn_PORT+1 ,(u8)(RJ45_1_Loc_Potr&0x00FF));     //设置本机端口号   低位
	RJ45_1_Write_Register(Sn_CR ,OPEN ); 
	if(RJ45_1_Read_Register(Sn_SR)!=SOCK_INIT)
	{
		i++;
		if(i<10)
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			goto Init1;
		}
		else
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			return Fail;
		}
	}
	RJ45_1_Write_Register(Sn_CR ,LISTEN);
	if(RJ45_1_Read_Register(Sn_SR)!=SOCK_LISTEN)
	{
		i++;
		if(i<10)
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			goto Init1;
		}
		else
		{
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			return Fail;
		}
	}
	return Success;
}

void RJ45_1_Read(u8 *RData)
{
	
	u16 Len,Start_Address,i,RD_Num,DataLength;
	RData[0]=RJ45_1_Read_Register(Sn_Rx_RSR);
	RData[1]=RJ45_1_Read_Register(Sn_Rx_RSR+1);  //读取接收到的字节长度

	Len=(u16)(RData[0]<<8)+RData[1];

	Len=Len&Sn_RX_Mask;

	Start_Address=RJ45_1_Read_Register(Sn_RX_RD);
	Start_Address=(Start_Address&0x00FF)<<8;	
	Start_Address+=RJ45_1_Read_Register(Sn_RX_RD+1); //读取初始地址
	RD_Num =Start_Address +Len;

	Start_Address=Start_Address&Sn_RX_Mask;
	if(Start_Address+Len>Sn_RX_Mask+1)
	{
		DataLength = Sn_RX_Mask+1-Start_Address;
		Start_Address+=Sn_RX_Base;			
		RJ45_1_CS_Low ;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
		RJ45_1_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RData[i+2]=RJ45_1_ReadWriteByte(0x00);
		}
		Start_Address =Sn_RX_Base ;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x00|(((Len-DataLength)&0x7F00)>>8));
		RJ45_1_ReadWriteByte((Len-DataLength)&0x00FF);
		for(i=0;i<(Len-DataLength);i++)
		{
			RData[i+DataLength+2]=RJ45_1_ReadWriteByte(0x00);
		}
		RJ45_1_CS_High ;	
	}
	else
	{
		Start_Address+=Sn_RX_Base;
		DataLength = Len;
		RJ45_1_CS_Low ;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
		RJ45_1_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RData[i+2]=RJ45_1_ReadWriteByte(0x00);
		}
		RJ45_1_CS_High ;	
	}	
	RJ45_1_Write_Register(Sn_RX_RD ,(RD_Num&0xFF00)>>8); 			  
	RJ45_1_Write_Register(Sn_RX_RD+1 ,RD_Num&0x00FF);	
	RJ45_1_Write_Register(Sn_CR ,RECV); 
	while(RJ45_1_Read_Register(Sn_CR));
}

void RJ45_1_Write(u8 *WData,u16 Len)
{
	u16 i,Start_Address,DataLength,WR_Num;
	Start_Address =(RJ45_1_Read_Register(Sn_TX_WR))<<8;
	Start_Address +=RJ45_1_Read_Register(Sn_TX_WR+1);
	WR_Num = Start_Address+Len;
	Start_Address =Start_Address&Sn_TX_Mask;
	if(Start_Address+Len>Sn_TX_Mask+1)
	{
		DataLength =Sn_TX_Mask +1-Start_Address;
		Start_Address+=Sn_TX_Base;
		RJ45_1_CS_Low ;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
		RJ45_1_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RJ45_1_ReadWriteByte(WData[i]);
		}
		Start_Address = Sn_TX_Base;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x80|(((Len-DataLength)&0x7F00)>>8));
		RJ45_1_ReadWriteByte((Len-DataLength)&0x00FF);
		for(i=0;i<Len-DataLength;i++)
		{
			RJ45_1_ReadWriteByte(WData[DataLength+i]);
		}
		RJ45_1_CS_High ;
		RJ45_1_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
		RJ45_1_Write_Register(Sn_TX_WR+1,(WR_Num&0x00FF));
	}
	else
	{
		Start_Address =Start_Address &Sn_TX_Mask;
		DataLength =Len;
		Start_Address+=Sn_TX_Base;
		RJ45_1_CS_Low ;
		RJ45_1_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_1_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_1_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
		RJ45_1_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RJ45_1_ReadWriteByte(WData[i]);
		}
		RJ45_1_CS_High ;
		RJ45_1_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
		RJ45_1_Write_Register(Sn_TX_WR+1,WR_Num&0x00FF);
	}
	RJ45_1_Write_Register(Sn_CR ,SEND);
	while(RJ45_1_Read_Register(Sn_CR));
}

//写数据到相关寄存器
void RJ45_1_Write_Register(u16 Address,u8 Data)
{
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
	RJ45_1_ReadWriteByte((Address&0x00FF));     // Address byte 2
	RJ45_1_ReadWriteByte(0x80);                 // Data write command and Write data length 1
	RJ45_1_ReadWriteByte(0x01);                 // Write data length 2
	RJ45_1_ReadWriteByte(Data);                 // Data write (write 1byte data)
	RJ45_1_CS_High ;
}

//读取相关寄存器数据
u8 RJ45_1_Read_Register(u16 Address)
{
	u8 Data;
	RJ45_1_CS_Low ;
	RJ45_1_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
	RJ45_1_ReadWriteByte((Address&0x00FF));     // Address byte 2
	RJ45_1_ReadWriteByte(0x00);                 // Data Read command and Write data length 1
	RJ45_1_ReadWriteByte(0x01);                 // Write data length 2
	Data=RJ45_1_ReadWriteByte(0x00);            // Data write (write 1byte data)
	RJ45_1_CS_High ;	
	return Data;
}

//SPI2数据读写
u8  RJ45_1_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  	
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个byte  数据
		
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte   
	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据	
}

//外部中断处理函数
void EXTI15_10_IRQHandler(void)
{
	u16 len;
	u8 StateFlag;
	u8 i;
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		StateFlag =RJ45_1_Read_Register(Sn_IR);
    if(StateFlag&CON)
		{		
			RJ45_1_Write_Register(Sn_IR ,CON);
			RJ45_1_Connect=1;
			LED5 =1;
		}
		if(StateFlag&DISCON_Flag)
		{
			RJ45_1_Write_Register(Sn_IR ,DISCON_Flag);
			RJ45_1_Write_Register(Sn_CR ,CLOSE); 
			LED5=0;
			RJ45_1_Connect=0;
		}
		if(StateFlag&RECEV)
		{	
			RJ45_1_Read(RJ45_1_RData);
			RJ45_1_ReceiveFlag=1;
			RJ45_1_Write_Register(Sn_IR,RECEV);		
		}
		EXTI_ClearITPendingBit(EXTI_Line14);//清除LINE15上的中断标志位 
	} 
}

/********************RJ45_2部分*****************************/
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
void RJ45_2_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	//RST
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_2);//拉高，不复位
	
	//PWDN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//拉低，正常工作模式
	
	//SPI1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI2时钟
	//PA4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_4);//拉高，失能SPI传输
	
	//引脚复用
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PA5~7复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); //PA5复用为 SPI1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1); //PA6复用为 SPI1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); //PA7复用为 SPI1
	
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器	
	SPI_Cmd(SPI1, ENABLE);
	
	//INT
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//RST对应IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);//PE14 连接到中断线14
	
	/* 配置EXTI_Line5 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;//LINE5
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE5
	EXTI_Init(&EXTI_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断10
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置	
}
	
void RJ45_2_Reset(void)
{
	uint16_t i,j;
	RJ45_2_RST_Low ;
	for(i=0;i<165;i++);
	RJ45_2_RST_High ;
	for(i=0;i<5000;i++)
	{
		for(j=0;j<1000;j++);
	}
}

void RJ45_2_Init(void)
{
	u8 i;
	RJ45_2_GPIO_Init();
	RJ45_2_Reset();
	//设置MAC ADDRESS
	for(i=0;i<6;i++)
	{
		RJ45_2_Write_Register(SHAR+i,RJ45_2_MAC[i]);
	}
	//设置Subnet Mask ADDRESS
	for(i=0;i<4;i++)
	{	
		RJ45_2_Write_Register(SUBR+i,RJ45_2_SubNet[i]);
	}
	//设置GateWay ADDRESS
	for(i=0;i<4;i++)
	{
		RJ45_2_Write_Register(GAR+i,RJ45_2_GateWay[i]);	
	}
	//设置Local IP ADDRESS
	for(i=0;i<4;i++)
	{
		RJ45_2_Write_Register(SIPR+i,RJ45_2_IP[i]);		
	}
	//设置重新发送时间以及重发次数
	RJ45_2_Write_Register(RTR,(u8)((RTR_Time&0xFF00)>>8));
	RJ45_2_Write_Register(RTR+1,(u8)(RTR_Time&0x00FF));
	RJ45_2_Write_Register(RCR,RCR_Num);
	//设置各Socket发送、接收最大数据包
	for(i=0;i<8;i++)
	{
		RJ45_2_Write_Register(Sn_RXMEM_SIZE+i*0x0100,W5200_Rx_Size[i]);
		RJ45_2_Write_Register(Sn_TXMEM_SIZE+i*0x0100,W5200_Tx_Size[i]);
	}	
}

u8 RJ45_2_TCP_Init(void)
{
	u8 i;
	RJ45_2_Write_Register(W5200_IMR,1<<0);                     //允许SOCKET0产生中断
	RJ45_2_Write_Register(Sn_IMR ,(u8)(RECEV|DISCON_Flag|CON)); //设置中断屏蔽寄存器
	Init1:	RJ45_2_Write_Register(Sn_MR ,TCP_MODE);                    //Set TCP mode
	RJ45_2_Write_Register(Sn_PORT ,(u8)((RJ45_2_Loc_Potr&0xFF00)>>8));  //设置本机端口号   高位
	RJ45_2_Write_Register(Sn_PORT+1 ,(u8)(RJ45_2_Loc_Potr&0x00FF));     //设置本机端口号   低位
	RJ45_2_Write_Register(Sn_CR ,OPEN ); 
	if(RJ45_2_Read_Register(Sn_SR)!=SOCK_INIT)
	{
		i++;
		if(i<10)
		{
			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
			goto Init1;
		}
		else
		{
			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
			return Fail;
		}
	}
	RJ45_2_Write_Register(Sn_CR ,LISTEN);
	if(RJ45_2_Read_Register(Sn_SR)!=SOCK_LISTEN)
	{
		i++;
		if(i<10)
		{
			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
			goto Init1;
		}
		else
		{
			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
			return Fail;
		}
	}
	return Success;
}

void RJ45_2_Read(u8 *RData)
{
	
	u16 Len,Start_Address,i,RD_Num,DataLength;
	RData[0]=RJ45_2_Read_Register(Sn_Rx_RSR);
	RData[1]=RJ45_2_Read_Register(Sn_Rx_RSR+1);  //读取接收到的字节长度

	Len=(u16)(RData[0]<<8)+RData[1];

	Len=Len&Sn_RX_Mask;

	Start_Address=RJ45_2_Read_Register(Sn_RX_RD);
	Start_Address=(Start_Address&0x00FF)<<8;	
	Start_Address+=RJ45_2_Read_Register(Sn_RX_RD+1); //读取初始地址
	RD_Num =Start_Address +Len;

	Start_Address=Start_Address&Sn_RX_Mask;
	if(Start_Address+Len>Sn_RX_Mask+1)
	{
		DataLength = Sn_RX_Mask+1-Start_Address;
		Start_Address+=Sn_RX_Base;			
		RJ45_2_CS_Low ;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
		RJ45_2_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RData[i+2]=RJ45_2_ReadWriteByte(0x00);
		}
		Start_Address =Sn_RX_Base ;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x00|(((Len-DataLength)&0x7F00)>>8));
		RJ45_2_ReadWriteByte((Len-DataLength)&0x00FF);
		for(i=0;i<(Len-DataLength);i++)
		{
			RData[i+DataLength+2]=RJ45_2_ReadWriteByte(0x00);
		}
		RJ45_2_CS_High ;	
	}
	else
	{
		Start_Address+=Sn_RX_Base;
		DataLength = Len;
		RJ45_2_CS_Low ;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x00|((DataLength&0x7F00)>>8));
		RJ45_2_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RData[i+2]=RJ45_2_ReadWriteByte(0x00);
		}
		RJ45_2_CS_High ;	
	}	
	RJ45_2_Write_Register(Sn_RX_RD ,(RD_Num&0xFF00)>>8); 			  
	RJ45_2_Write_Register(Sn_RX_RD+1 ,RD_Num&0x00FF);	
	RJ45_2_Write_Register(Sn_CR ,RECV); 
	while(RJ45_2_Read_Register(Sn_CR));
}

void RJ45_2_Write(u8 *WData,u16 Len)
{
	u16 i,Start_Address,DataLength,WR_Num;
	Start_Address =(RJ45_2_Read_Register(Sn_TX_WR))<<8;
	Start_Address +=RJ45_2_Read_Register(Sn_TX_WR+1);
	WR_Num = Start_Address+Len;
	Start_Address =Start_Address&Sn_TX_Mask;
	if(Start_Address+Len>Sn_TX_Mask+1)
	{
		DataLength =Sn_TX_Mask +1-Start_Address;
		Start_Address+=Sn_TX_Base;
		RJ45_2_CS_Low ;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
		RJ45_2_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RJ45_2_ReadWriteByte(WData[i]);
		}
		Start_Address = Sn_TX_Base;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x80|(((Len-DataLength)&0x7F00)>>8));
		RJ45_2_ReadWriteByte((Len-DataLength)&0x00FF);
		for(i=0;i<Len-DataLength;i++)
		{
			RJ45_2_ReadWriteByte(WData[DataLength+i]);
		}
		RJ45_2_CS_High ;
		RJ45_2_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
		RJ45_2_Write_Register(Sn_TX_WR+1,(WR_Num&0x00FF));
	}
	else
	{
		Start_Address =Start_Address &Sn_TX_Mask;
		DataLength =Len;
		Start_Address+=Sn_TX_Base;
		RJ45_2_CS_Low ;
		RJ45_2_ReadWriteByte((Start_Address&0xFF00)>>8);  // Address byte 1
		RJ45_2_ReadWriteByte((Start_Address&0x00FF));     // Address byte 2
		RJ45_2_ReadWriteByte(0x80|((DataLength&0x7F00)>>8));
		RJ45_2_ReadWriteByte(DataLength&0x00FF);
		for(i=0;i<DataLength;i++)
		{
			RJ45_2_ReadWriteByte(WData[i]);
		}
		RJ45_2_CS_High ;
		RJ45_2_Write_Register(Sn_TX_WR,(WR_Num&0xFF00)>>8);
		RJ45_2_Write_Register(Sn_TX_WR+1,WR_Num&0x00FF);
	}
	RJ45_2_Write_Register(Sn_CR ,SEND);
	while(RJ45_2_Read_Register(Sn_CR));
}

void RJ45_2_Write_Register(u16 Address,u8 Data)
{
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
	RJ45_2_ReadWriteByte((Address&0x00FF));     // Address byte 2
	RJ45_2_ReadWriteByte(0x80);                 // Data write command and Write data length 1
	RJ45_2_ReadWriteByte(0x01);                 // Write data length 2
	RJ45_2_ReadWriteByte(Data);                 // Data write (write 1byte data)
	RJ45_2_CS_High ;	
}

//读取相关寄存器数据
u8 RJ45_2_Read_Register(u16 Address)
{
	u8 Data;
	RJ45_2_CS_Low ;
	RJ45_2_ReadWriteByte((Address&0xFF00)>>8);  // Address byte 1
	RJ45_2_ReadWriteByte((Address&0x00FF));     // Address byte 2
	RJ45_2_ReadWriteByte(0x00);                 // Data Read command and Write data length 1
	RJ45_2_ReadWriteByte(0x01);                 // Write data length 2
	Data=RJ45_2_ReadWriteByte(0x00);                 // Data write (write 1byte data)
	RJ45_2_CS_High ;
	return Data;
}

//SPI1数据读写
u8  RJ45_2_ReadWriteByte(u8 TxData)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  	
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个byte  数据
		
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte   
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据	
}


//外部中断处理函数
void EXTI9_5_IRQHandler(void)
{
	u16 len;
	u8 i,EventInformation;
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EventInformation=RJ45_2_Read_Register(Sn_IR);
    if(EventInformation&CON)
		{
			RJ45_2_Connect =1 ;
		  LED4 =1;
			RJ45_2_Write_Register(Sn_IR ,CON);
		}
		if(EventInformation&DISCON_Flag)
		{
			RJ45_2_Write_Register(Sn_IR ,DISCON_Flag);
			RJ45_2_Write_Register(Sn_CR ,CLOSE); 
			LED4 =0;	
			RJ45_2_Init();
			delay_ms(100);
			RJ45_2_TCP_Init();			
		}
		if(EventInformation&RECEV)
		{	
			RJ45_2_Read(RJ45_2_RData);
			RJ45_2_ReceiveFlag=1;
			RJ45_2_Write_Register(Sn_IR,RECEV);		
		}
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
}


