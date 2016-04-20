#include "AT24C02.h"
#include "delay.h"
extern volatile u8 Device;  
extern u8 RJ45_1_IP[4];
extern u8 RJ45_2_IP[4];
extern u16 RJ45_1_Loc_Potr;
extern u16 RJ45_2_Loc_Potr;
/********** IIC引脚初始化*****************/
/*-------------|----------------
	        PD12<->SCL
					PD13<->SDA
  -------------|----------------*/
void AT24C02_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOB时钟
  //GPIOD12,D13初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化
	AT24C02_SCL=1;
	AT24C02_SDA=1;
}
void AT24C02_Init(void)
{
	AT24C02_GPIO_Init();
}
//设置设备信息
void SetDeviceInformation(u8 *Information,u8 Length)    
{
	u8 i;
	u8 j;
	for(i=0;i<Length;i++)
	{
		AT24C02_WriteOneByte(i,Information[i]);
		for(j=0;j<100;j++);
	}
}
void GetDeviceInformation(void)
{
	Device =AT24C02_ReadOneByte (0x00);
	RJ45_1_Loc_Potr =AT24C02_ReadOneByte (0x01);
	RJ45_1_Loc_Potr<<=8;
	RJ45_1_Loc_Potr+=AT24C02_ReadOneByte (0x02);
	AT24C02_Read (0x03,RJ45_1_IP,4);
	RJ45_2_Loc_Potr =AT24C02_ReadOneByte (0x07);
	RJ45_2_Loc_Potr<<=8;
	RJ45_2_Loc_Potr+=AT24C02_ReadOneByte (0x08);
	AT24C02_Read (0x09,RJ45_2_IP,4);
}
//在AT24C02指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 AT24C02_ReadOneByte(u8 ReadAddr)
{
	u8 RxData;
	AT24C02_IIC_Start();
	AT24C02_Send_Byte(0xA0);
	AT24C02_IIC_Wait_Ack();
	AT24C02_Send_Byte(ReadAddr);   //发送地址
	AT24C02_IIC_Wait_Ack();
	AT24C02_IIC_Start();
	AT24C02_Send_Byte(0xA1);      //接收指令
	AT24C02_IIC_Wait_Ack();
	RxData=AT24C02_Read_Byte(0);
	AT24C02_IIC_Stop();
	return RxData;	
}

//在AT24C02指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24C02_WriteOneByte(u8 WriteAddr,u8 DataToWrite)
{
	u32 i;
	AT24C02_IIC_Start();
	AT24C02_Send_Byte(0xA0);
  AT24C02_IIC_Wait_Ack();
	AT24C02_Send_Byte(WriteAddr);   //发送地址
	AT24C02_IIC_Wait_Ack();
	AT24C02_Send_Byte(DataToWrite);
	AT24C02_IIC_Wait_Ack();
	AT24C02_IIC_Stop();	
	for(i=0;i<1000000;i++);
}

//在AT24C02里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24C02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24C02_Read(u8 ReadAddr,u8 *pBuffer,u8 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24C02_ReadOneByte(ReadAddr++);
		NumToRead--;
	}
}

//在AT24C02里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24C02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24C02_Write(u8 WriteAddr,u8 *pBuffer,u8 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24C02_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

//IIC开始信号
void AT24C02_IIC_Start(void)
{
	u8 i;
	AT24C02_SDA_OUT();
	AT24C02_SDA=1;
	AT24C02_SCL=1;
	for(i=0;i<50;i++);
	AT24C02_SDA=0;
	for(i=0;i<50;i++);
	AT24C02_SCL=0;
}

//IIC结束信号
void AT24C02_IIC_Stop(void)
{
	u8 i;
	AT24C02_SDA_OUT();
	AT24C02_SDA=0;
	AT24C02_SCL=0;
  for(i=0;i<50;i++);
	AT24C02_SCL=1;
	AT24C02_SDA=1;
  for(i=0;i<50;i++);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 AT24C02_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0,i=0;
	AT24C02_SDA_IN();
	AT24C02_SDA=1;
	for(i=0;i<20;i++); 
	AT24C02_SCL=1;
	for(i=0;i<20;i++); 	 
	while(AT24C02_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			AT24C02_IIC_Stop();
			return 1;
		}
	}
	AT24C02_SCL=0;//时钟输出0 	   
	return 0;  
}

//产生ACK应答
void AT24C02_IIC_Ack(void)
{
	u8 i;
	AT24C02_SCL=0;
	AT24C02_SDA_OUT();
	AT24C02_SDA=0;
	for(i=0;i<20;i++); 
	AT24C02_SCL=1;
	for(i=0;i<20;i++); 
	AT24C02_SCL=0;
}

//不产生ACK应答		    
void AT24C02_IIC_NAck(void)
{
	u8 i;
	AT24C02_SCL=0;
	AT24C02_SDA_OUT();
	for(i=0;i<20;i++); 
	AT24C02_SCL=1;
	for(i=0;i<20;i++); 
	AT24C02_SCL=0;
}

//AT24C02发送一个字节
void AT24C02_Send_Byte(u8 TxData)
{                        
  u8 t,i;   
	AT24C02_SDA_OUT(); 	    
  AT24C02_SCL=0;//拉低时钟开始数据传输
  for(t=0;t<8;t++)
  {              
    AT24C02_SDA=(TxData&0x80)>>7;
    TxData<<=1; 
    for(i=0;i<20;i++); 	   
		AT24C02_SCL=1;
		for(i=0;i<20;i++); 	 
		AT24C02_SCL=0;	
		for(i=0;i<20;i++); 
   }	 
} 

//AT24C02读取一个字节
u8 AT24C02_Read_Byte(u8 Ack)
{
	u8 i,j,RxData;
	AT24C02_SDA_IN();
	for(i=0;i<8;i++)
	{
		AT24C02_SCL=0;	
		for(j=0;j<20;j++); 
		AT24C02_SCL=1;
		RxData<<=1;
		RxData+=AT24C02_READ_SDA;	
		for(j=0;j<20;j++);
	}
	if(Ack)
		AT24C02_IIC_Ack();
	else
		AT24C02_IIC_NAck();	
	return RxData;
}
