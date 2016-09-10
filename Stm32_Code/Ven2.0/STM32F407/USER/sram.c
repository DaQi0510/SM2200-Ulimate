#include "sram.h"	       


u8 ad1_buffer[Max_Buffer]       __attribute__(( at(Bank1_SRAM3_ADDR) ));
u8 ad2_buffer[Max_Buffer]       __attribute__(( at(Bank1_SRAM3_ADDR+Max_Buffer) ));
u8 ad3_buffer[Max_Buffer]       __attribute__(( at(Bank1_SRAM3_ADDR+Max_Buffer*2) ));
u8 ad4_buffer[Max_Buffer]       __attribute__(( at(Bank1_SRAM3_ADDR+Max_Buffer*3) ));

/******************************************************************************/
/*                              SRAM初始化                                    */
/******************************************************************************/

void sram_init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	
	///////////////////////////////////////////////////////////////////////
	////                        SRAM GPIO配置                          ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE 
	| RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);//使能PD,PE,PF,PG时钟

	GPIO_InitStructure.GPIO_Pin = (3<<0)|(3<<4)|(0XFF<<8);//PD0,1,4,5,8~15 AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化  
	
  GPIO_InitStructure.GPIO_Pin = (3<<0)|(0X1FF<<7);//PE0,1,7~15,AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化  
	
 	GPIO_InitStructure.GPIO_Pin = (0X3F<<0)|(0XF<<12); 	//PF0~5,12~15
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化  

	GPIO_InitStructure.GPIO_Pin =(0X3F<<0)| GPIO_Pin_10;//PG0~5,10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化 
 
 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//PD0,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//PD1,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//PD15,AF12
 
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource1,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//PE7,AF12
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//PE15,AF12
 
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource0,GPIO_AF_FSMC);//PF0,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource1,GPIO_AF_FSMC);//PF1,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource2,GPIO_AF_FSMC);//PF2,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource3,GPIO_AF_FSMC);//PF3,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource4,GPIO_AF_FSMC);//PF4,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource5,GPIO_AF_FSMC);//PF5,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource12,GPIO_AF_FSMC);//PF12,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource13,GPIO_AF_FSMC);//PF13,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource14,GPIO_AF_FSMC);//PF14,AF12
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource15,GPIO_AF_FSMC);//PF15,AF12
	
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource0,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource1,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource2,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource3,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource5,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource10,GPIO_AF_FSMC);
	
	///////////////////////////////////////////////////////////////////////
	////                           FSMC配置                            ////
	///////////////////////////////////////////////////////////////////////
	
 	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//使能FSMC时钟  
	
 	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为1个HCLK 1/168M=6ns
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
  readWriteTiming.FSMC_DataSetupTime = 0x02;		 ////数据保持时间（DATAST）为2个HCLK 2*6=12ns	 	 
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
	
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;// 使用NE3
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit  
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//存储器写使能 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable; // 读写使用相同的时序
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &readWriteTiming; //读写同样时序

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

 	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);  // 使能BANK3										  
											
}
	  														  
/******************************************************************************/
/*                            SRAM写数据 8位                                  */
/******************************************************************************/

void sram_write_byte(u8* pBuffer,u32 WriteAddr,u32 n)
{
	WriteAddr+=Bank1_SRAM3_ADDR;
	for(;n!=0;n--)  
	{										    
		*(vu8*)WriteAddr=*pBuffer;	  
		WriteAddr++;
		pBuffer++;
	}   
}			

/******************************************************************************/
/*                            SRAM读数据 8位                                  */
/******************************************************************************/

void sram_read_byte(u8* pBuffer,u32 ReadAddr,u32 n)
{
	ReadAddr+=Bank1_SRAM3_ADDR;
	for(;n!=0;n--)  
	{											    
		*pBuffer=*(vu8*)ReadAddr;
		ReadAddr++;
		pBuffer++;
	}  
}

/******************************************************************************/
/*                            SRAM写数据 8位                                  */
/******************************************************************************/

void sram_write_word(u16* pBuffer,u32 WriteAddr,u32 n)
{
	WriteAddr+=Bank1_SRAM3_ADDR;
	for(;n!=0;n--)  
	{										    
		*(vu16*)WriteAddr=*pBuffer;	  
		WriteAddr+=2;
		pBuffer++;
	}   
}			

/******************************************************************************/
/*                            SRAM读数据 8位                                  */
/******************************************************************************/

void sram_read_word(u16* pBuffer,u32 ReadAddr,u32 n)
{
	ReadAddr+=Bank1_SRAM3_ADDR;
	for(;n!=0;n--)  
	{											    
		*pBuffer=*(vu16*)ReadAddr;
		ReadAddr+=2;
		pBuffer++;
	}  
}
