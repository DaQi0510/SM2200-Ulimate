#include "W5200.h"
#include "led.h"
#include "wwdg.h"
#include "sm2200.h"
#include "AT24C02.h"
extern u8  ConnectState;       //设备连接状态

extern volatile u8 ConnectDevice[7];     //连接设备号
extern volatile u8 DeviceScale;          //连接从属级别   0从 1主

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
extern u16 RJ45_1_RLength;
extern u8 RJ45_1_Connect;  //连接状态
extern u8 RJ45_1_ReceiveFlag;
extern u16 RJ45_1_Send;     //发送状态
extern u8 RJ45_1_DirIP[4];   //对方服务器IP地址
extern u16 RJ45_1_Dir_Port;   
extern u8 RJ45_2_MAC[6];
extern u8 RJ45_2_IP[4];
extern u8 RJ45_2_GateWay[4];
extern u8 RJ45_2_SubNet[4];
extern u16 RJ45_2_Loc_Potr;
extern u8 RJ45_2_RData[1024];
extern u8 RJ45_2_WData[1024];
extern u8 RJ45_2_RLength;
extern u8 RJ45_2_Connect;     //连接状态
extern u8 RJ45_2_ReceiveFlag; //标记网口2是否接收到数据
extern u8 RJ45_2_Send;        //发送状态
extern u8 Dest_IP[4] ;
extern u16 RTR_Time; //重新发送时间
extern u8 RCR_Num;   //重新发送次数 
u8 W5200_Rx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Rx_Size[i])K And SUM=16K
u8 W5200_Tx_Size[8]={0x08,0x02,0x01,0x01,0x01,0x01,0x01,0x01}; //Channel_Size[i]=(Tx_Size[i])K And SUM=16K
extern volatile u8 CommandFlag;
u16 RJ45_2_DataLength;
extern u8 ShakeHands[7];
extern u8 Flag;
extern u8 Len;
extern volatile u8 Netflag;   //用于指示当前网络所处状态
extern volatile u8 BER_Flag;

extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern u8 Voltage;                   //记录电压幅值
extern volatile u16 ToDevice;        //要发送到数据的设备号  
extern volatile u8 ReDevice;        //接收到数据的设备号

extern u8 RunMode;  //设备运行模式   轮询模式：1
/******上位机命令定义*********/
u8 Connect=0x00;      //建立连接命令 
u8 DisConnect=0x01;   //断开连接命令 
u8 PollChat=0x02;     //轮询通信配置主地址
u8 RunModes=0x03; //获取通信配置信息
u8 WriteTem[2];
u8 ReadTem[2];
/********************RJ45_1部分*****************************/
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOE,GPIO_Pin_12);//拉高，不复位
	
	
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
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置	
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
	//设置MAC ADDRESS
	RJ45_1_Write_Buf(SHAR0,RJ45_1_MAC,6);
	//设置Subnet Mask ADDRESS
	RJ45_1_Write_Buf(SUBR0,RJ45_1_SubNet,4);
	//设置GateWay ADDRESS
	RJ45_1_Write_Buf(GAR0,RJ45_1_GateWay,4);
	//设置Local IP ADDRESS
	RJ45_1_Write_Buf(SIPR0,RJ45_1_IP,4);

	//设置重新发送时间以及重发次数
	WriteTem[0]=RTR_Time/256;
	WriteTem[1]=RTR_Time%256;
	RJ45_1_Write_Buf(RTR0,WriteTem,2);
	RJ45_1_Write_Register(WIZ_RCR,RCR_Num);
	//设置各Socket发送、接收最大数据包
	for(i=0;i<8;i++)
	{
		RJ45_1_Write_Register(Sn_TXMEM_SIZE(i),W5200_Tx_Size[i]);
		RJ45_1_Write_Register(Sn_RXMEM_SIZE(i),W5200_Rx_Size[i]);
	}
}

//TCP模式初始化        服务器
u8 RJ45_1_TCP_ServiceInit(void)
{
	u8 i,j=0;
	RJ45_1_Write_Register(SIMR,1<<0); //允许SOCKET0产生中断                  
	RJ45_1_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_TIMEOUT|Sn_IR_DISCON|Sn_IR_CON); //设置中断屏蔽寄存器
	CloseSocket_RJ45_1();
	RJ45_1_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcp模式，无延时
	WriteTem[0]=RJ45_1_Loc_Potr/256;
	WriteTem[1]=RJ45_1_Loc_Potr%256;
	RJ45_1_Write_Buf(Sn_PORT0(0),WriteTem,2);      //设置端口号
	RJ45_1_Write_Register(Sn_KPALVTR(0),1);        //每5s自动检测一次连接状态
  Init1:	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_OPEN);
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	if(RJ45_1_Read_Register(Sn_SR(0))!=SOCK_INIT)   //检测网口开启状态
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
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_LISTEN);   //开启监听状态
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	return Success;
}
/**
*@brief		TCP模式初始化  客户端   默认连接三次，三次连接不上，返回Fail
*@param   无
*@return	无
*/
u8 RJ45_1_TCP_ClientInit(void)
{
	u8 i,j=0;
	RJ45_1_Write_Register(SIMR,1<<0); //允许SOCKET0产生中断                  
	RJ45_1_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_DISCON|Sn_IR_CON); //设置中断屏蔽寄存器
	CloseSocket_RJ45_1();
	RJ45_1_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcp模式，无延时
	WriteTem[0]=RJ45_1_Loc_Potr/256;
	WriteTem[1]=RJ45_1_Loc_Potr%256;
	RJ45_1_Write_Buf(Sn_PORT0(0),WriteTem,2);      //设置端口号
  WriteTem[0]=RJ45_1_Dir_Port/256;
	WriteTem[1]=RJ45_1_Dir_Port%256;
	RJ45_1_Write_Buf(Sn_DPORT0(0),WriteTem,2);     //设置目标服务器端口号
	RJ45_1_Write_Buf(Sn_DIPR0(0),RJ45_1_DirIP,4);  //目标服务器IP地址
	RJ45_1_Write_Register(Sn_KPALVTR(0),1);        //每5s自动检测一次连接状态
  Init1:	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_OPEN);
	for(i=0;i<20;i++);
	while(RJ45_1_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	if(RJ45_1_Read_Register(Sn_SR(0))!=SOCK_INIT)   //检测网口开启状态
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
	
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_CONNECT);   //开启连接
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
*@brief		读取数据
*@param   RData：读取数据存放数组
*@return	fail 0  success 1
*/
void RJ45_1_Read(u8 *RData)
{
	u16 Start_Address;
	u32 Address;
	RJ45_1_Read_Buf(Sn_RX_RSR0(0),ReadTem,2);
	RJ45_1_RLength =ReadTem[0]*256+ReadTem[1];     //获取接收到数据长度
	RJ45_1_Read_Buf(Sn_RX_RD0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];       //获取相对地址
	WriteTem[0]=(Start_Address+RJ45_1_RLength)/256;
	WriteTem[1]=(Start_Address+RJ45_1_RLength)%256;
	Address=(u32)(Start_Address<<8)+(0<<5)+0x18;   //得到绝对地址
	RJ45_1_Read_Buf(Address,RData,RJ45_1_RLength); //读取数据
	RJ45_1_Write_Buf(Sn_RX_RD0(0),WriteTem,2);      //写入读取指针
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_RECV);
	while(RJ45_1_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}
/**
*@brief		发送数据
*@param   WData：发送数组
*@param   Len：  发送数据长度,Len<=8K
*@return	无
*/
void RJ45_1_Write(u8 *WData,u16 Len)
{
	u16 Start_Address;
	u32 Address;
	RJ45_1_Read_Buf(Sn_TX_WR0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];      //获取写入地址指针
	Address=(u32)(Start_Address<<8)+(0<<5)+0x10;  //计算绝对地址
	RJ45_1_Write_Buf(Address,WData,Len);          //写入缓存
	Start_Address+=Len;
	WriteTem[0]=Start_Address/256;
	WriteTem[1]=Start_Address%256;
	RJ45_1_Write_Buf(Sn_TX_WR0(0),WriteTem,2);    //更新指针
	RJ45_1_Write_Register(Sn_CR(0),Sn_CR_SEND);   //发送
	while(RJ45_1_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}

/**
*@brief		关闭网络
*@return	无
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
*@brief		向W5500写入一个8位数据
*@param		Address: 写入数据的地址
*@param   Data：写入的数据
*@return	无
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
*@brief		向W5500写入len字节数据
*@param		Address: 写入数据的地址
*@param   Buf：写入字节地址
*@param   Len：写入字节长度
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
*@brief		从W5500读出一个8位数据
*@param		Address: 写入数据的地址
*@return Data：从写入的地址处读取到的8位数据
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
*@brief		从W5500读出Len字节数据
*@param		Address: 读取数据的地址
*@param 	Buf：存放读取数据
*@param		len：字节长度
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
//    if(StateFlag&Sn_IR_TIMEOUT)  //连接超时
//		{
//			RJ45_1_Write_Register(Sn_IR(0),Sn_IR_TIMEOUT);
//			CloseSocket_RJ45_1();
//		}
		EXTI_ClearITPendingBit(EXTI_Line14);//清除LINE15上的中断标志位 
	} 
}

/********************RJ45_2部分*****************************/
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
	GPIO_SetBits(GPIOA,GPIO_Pin_2);//拉高，不复位
	
	
	//SPI2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOB时钟
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
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PB13~15复用功能输出	
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//INT对应IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);//PC5 连接到中断线5
	
	/* 配置EXTI_Line14 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;//LINE5
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE10
	EXTI_Init(&EXTI_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断10
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置	
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
	//设置MAC ADDRESS
	RJ45_2_Write_Buf(SHAR0,RJ45_2_MAC,6);
	//设置Subnet Mask ADDRESS
	RJ45_2_Write_Buf(SUBR0,RJ45_2_SubNet,4);
	//设置GateWay ADDRESS
	RJ45_2_Write_Buf(GAR0,RJ45_2_GateWay,4);
	//设置Local IP ADDRESS
	RJ45_2_Write_Buf(SIPR0,RJ45_2_IP,4);

	//设置重新发送时间以及重发次数
	WriteTem[0]=RTR_Time/256;
	WriteTem[1]=RTR_Time%256;
	RJ45_2_Write_Buf(RTR0,WriteTem,2);
	RJ45_2_Write_Register(WIZ_RCR,RCR_Num);
	//设置各Socket发送、接收最大数据包
	for(i=0;i<8;i++)
	{
		RJ45_2_Write_Register(Sn_TXMEM_SIZE(i),W5200_Tx_Size[i]);
		RJ45_2_Write_Register(Sn_RXMEM_SIZE(i),W5200_Rx_Size[i]);
	}
}

//TCP模式初始化        服务器
u8 RJ45_2_TCP_ServiceInit(void)
{
	u8 i,j=0;
	RJ45_2_Write_Register(SIMR,1<<0); //允许SOCKET0产生中断                  
	RJ45_2_Write_Register(Sn_IMR(0) ,Sn_IR_RECV|Sn_IR_TIMEOUT|Sn_IR_DISCON|Sn_IR_CON); //设置中断屏蔽寄存器
	CloseSocket_RJ45_2();
	RJ45_2_Write_Register(Sn_MR(0),Sn_MR_TCP|Sn_MR_ND);      //tcp模式，无延时
	WriteTem[0]=RJ45_2_Loc_Potr/256;
	WriteTem[1]=RJ45_2_Loc_Potr%256;
	RJ45_2_Write_Buf(Sn_PORT0(0),WriteTem,2);      //设置端口号
	RJ45_2_Write_Register(Sn_KPALVTR(0),1);        //每5s自动检测一次连接状态
  Init1:	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_OPEN);
	for(i=0;i<20;i++);
	while(RJ45_2_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	if(RJ45_2_Read_Register(Sn_SR(0))!=SOCK_INIT)   //检测网口开启状态
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
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_LISTEN);   //开启监听状态
	for(i=0;i<20;i++);
	while(RJ45_2_Read_Register(Sn_CR(0)))   	/*Wait to process the command*/
	{
		for(i=0;i<20;i++);       
	}
	return Success;
}
/**
*@brief		读取数据
*@param   RData：读取数据存放数组
*@return	无
*/
void RJ45_2_Read(u8 *RData)
{
	u16 Start_Address;
	u32 Address;
	RJ45_2_Read_Buf(Sn_RX_RSR0(0),ReadTem,2);
	RJ45_2_RLength =ReadTem[0]*256+ReadTem[1];     //获取接收到数据长度
	RJ45_2_Read_Buf(Sn_RX_RD0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];       //获取相对地址
	WriteTem[0]=(Start_Address+RJ45_2_RLength)/256;
	WriteTem[1]=(Start_Address+RJ45_2_RLength)%256;
	Address=(u32)(Start_Address<<8)+(0<<5)+0x18;   //得到绝对地址
	RJ45_2_Read_Buf(Address,RData,RJ45_2_RLength); //读取数据
	RJ45_2_Write_Buf(Sn_RX_RD0(0),WriteTem,2);      //写入读取指针
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_RECV);
	while(RJ45_2_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}
/**
*@brief		发送数据
*@param   WData：发送数组
*@param   Len：  发送数据长度,Len<=8K
*@return	无
*/
void RJ45_2_Write(u8 *WData,u16 Len)
{
	u16 Start_Address;
	u32 Address;
	RJ45_2_Read_Buf(Sn_TX_WR0(0),ReadTem,2);
	Start_Address=ReadTem[0]*256+ReadTem[1];      //获取写入地址指针
	Address=(u32)(Start_Address<<8)+(0<<5)+0x10;  //计算绝对地址
	RJ45_2_Write_Buf(Address,WData,Len);          //写入缓存
	Start_Address+=Len;
	WriteTem[0]=Start_Address/256;
	WriteTem[1]=Start_Address%256;
	RJ45_2_Write_Buf(Sn_TX_WR0(0),WriteTem,2);    //更新指针
	RJ45_2_Write_Register(Sn_CR(0),Sn_CR_SEND);   //发送
	while(RJ45_2_Read_Register(Sn_CR(0)));   	/*Wait to process the command*/
}

/**
*@brief		关闭网络
*@return	无
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
*@brief		向W5500写入一个8位数据
*@param		Address: 写入数据的地址
*@param   Data：写入的数据
*@return	无
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
*@brief		向W5500写入len字节数据
*@param		Address: 写入数据的地址
*@param   Buf：写入字节地址
*@param   Len：写入字节长度
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
*@brief		从W5500读出一个8位数据
*@param		Address: 写入数据的地址
*@return Data：从写入的地址处读取到的8位数据
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
*@brief		从W5500读出Len字节数据
*@param		Address: 读取数据的地址
*@param 	Buf：存放读取数据
*@param		len：字节长度
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
void RJ45_2_Deal(void)     //接收数据处理
{
  u8 i;
	if((RJ45_2_RData[0]==0x3C)&&(RJ45_2_RData[RJ45_2_RLength-1]==0x3E))    //判断数据包头包尾
	{
		if(RJ45_2_RData[1]==Connect)  //与电脑建立连接
		{
				RJ45_2_WData[0]=0x3C;
				RJ45_2_WData[1]=Connect;
			  RJ45_2_WData[7]=0x3E;
				RJ45_2_Write(RJ45_2_WData,8);
			  RJ45_2_Connect=1;
		}
		if(RJ45_2_RData[1]==DisConnect)  //与电脑断开连接
		{
				RJ45_2_WData[0]=0x3C;
				RJ45_2_WData[1]=DisConnect;
			  RJ45_2_WData[7]=0x3E;
				RJ45_2_Write(RJ45_2_WData,8);
			  RJ45_2_Connect=0;
		}
		if(RJ45_2_RData[1]==PollChat)  //轮询通信部分
		{
			if(RJ45_2_RData[2]==0x00)   //通信配置部分
			{
				/*******读取配置信息***********/
				DeviceScale =RJ45_2_RData[3];
				for(i=0;i<7;i++)
				{
					ConnectDevice[i]=RJ45_2_RData[4+i];
				}
				Voltage=RJ45_2_RData[11];
				OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);//以为着最多可开2~3个通道
				/******保存配置信息**********/
				AT24C02_WriteOneByte(0x13,DeviceScale);   //设备主从模式
				AT24C02_WriteOneByte(0x14,Voltage);   //设备主从模式
				for(i=0;i<7;i++)
				{
					AT24C02_WriteOneByte(0x15+i,ConnectDevice[i]);
				}
				/*****发送应答信息**********/
				RJ45_2_Write(RJ45_2_RData,16);
			}
			if(RJ45_2_RData[2]==0x01)   //通信配置读取部分
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
		if(RJ45_2_RData[1]==RunModes)
		{
			//读取设备运行模式
			RunMode =RJ45_2_RData[2];
			//保存运行模式
			AT24C02_WriteOneByte(0x20,RunMode);
			//发送应答
			RJ45_2_Write(RJ45_2_RData,8);
		}
//    if(RJ45_2_RData[1]==0x02)  //发送信息确认
//		{
//		}
//		if(RJ45_2_RData[1]==0x03)  //接收信息确认
//		{
//		}
//		if(RJ45_2_RData[1]==0x04)  //通信频点查找
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
//void Message_Send(void)    //将发送信息反馈到电脑
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
//		NVIC_EXTI(0);   //屏蔽所有外部中断
//		RJ45_2_Write(RJ45_2_WData,42);
//		NVIC_EXTI(1);  //允许所有外部中断
//	}
//}
//void Message_Rece(void)    //将接收信息反馈到电脑
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
//		NVIC_EXTI(0);   //屏蔽所有外部中断
//		RJ45_2_Write(RJ45_2_WData,42);
//		NVIC_EXTI(1);  //允许所有外部中断
//	}
//}


