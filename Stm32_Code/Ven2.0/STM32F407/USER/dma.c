#include "dma.h"

/******************************************************************************/
/*                              DMA2初始化                                    */
/*                   SPI1_TX--DMA2--CHANNEL3--STREAM3                         */
/*                   SPI1_RX--DMA2--CHANNEL3--STREAM2                         */
/******************************************************************************/

unsigned char spi1_rx[6], spi1_tx[6];
void dma2_init(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	///////////////////////////////////////////////////////////////////////
	////                     SPI1_TX DMA配置                           ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
	DMA_DeInit(DMA2_Stream3);
	
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI1->DR;//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)spi1_tx;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
	DMA_InitStructure.DMA_BufferSize = 6;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//最高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);//初始化DMA Stream
	SPI_I2S_DMACmd( SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能SPI1_TX DMA传输 

	
	///////////////////////////////////////////////////////////////////////
	////                     SPI1_RX DMA配置                           ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
	DMA_DeInit(DMA2_Stream2);
	
	while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI1->DR;//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)spi1_rx;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
	DMA_InitStructure.DMA_BufferSize = 6;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//最高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);//开启DMA2_Stream2传输完成中断
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);//初始化DMA Stream
	SPI_I2S_DMACmd( SPI1, SPI_I2S_DMAReq_Rx, ENABLE);//使能SPI1_TX DMA传输
	
	///////////////////////////////////////////////////////////////////////
	////                        NVIC配置                               ////
	///////////////////////////////////////////////////////////////////////
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

/******************************************************************************/
/*                              DMA1初始化                                    */
/*                   SPI1_TX--DMA2--CHANNEL0--STREAM4                         */
/*                   SPI1_RX--DMA2--CHANNEL0--STREAM3                         */
/******************************************************************************/

unsigned char spi2_rx[6], spi2_tx[6];
void dma1_init(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	///////////////////////////////////////////////////////////////////////
	////                     SPI2_TX DMA配置                           ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	DMA_DeInit(DMA1_Stream4);
	
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)spi2_tx;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
	DMA_InitStructure.DMA_BufferSize = 6;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//最高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);//初始化DMA Stream
	SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Tx, ENABLE);//使能SPI2_TX DMA传输 

	
	///////////////////////////////////////////////////////////////////////
	////                     SPI2_RX DMA配置                           ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA2时钟使能 
	DMA_DeInit(DMA1_Stream3);
	
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)spi2_rx;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
	DMA_InitStructure.DMA_BufferSize = 6;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//最高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);//开启DMA2_Stream2传输完成中断
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);//初始化DMA Stream
	SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Rx, ENABLE);//使能SPI2_TX DMA传输
	
	///////////////////////////////////////////////////////////////////////
	////                        NVIC配置                               ////
	///////////////////////////////////////////////////////////////////////
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

/******************************************************************************/
/*                        SPI1 TX DMA传输一次                                 */
/******************************************************************************/

void spi1_tx_dma_transfer_once(void)
{
	DMA2->LIFCR=(1<<27);//清除TCIF3标志
	DMA2_Stream3->CR|=1<<0; //开启DMA传输
}

/******************************************************************************/
/*                       SPI1 RX DMA传输一次                                  */
/******************************************************************************/

void spi1_rx_dma_transfer_once(void)
{
	DMA2->LIFCR=(1<<21);//清除TCIF2标志
	DMA2_Stream2->CR|=1<<0; //开启DMA传输
}

/******************************************************************************/
/*                        SPI2 TX DMA传输一次                                 */
/******************************************************************************/

void spi2_tx_dma_transfer_once(void)
{
	DMA1->HIFCR=(1<<5);//清除TCIF4标志
	DMA1_Stream4->CR|=1<<0; //开启DMA传输
}

/******************************************************************************/
/*                       SPI2 RX DMA传输一次                                  */
/******************************************************************************/

void spi2_rx_dma_transfer_once(void)
{
	DMA1->LIFCR=(1<<27);//清除TCIF3标志
	DMA1_Stream3->CR|=1<<0; //开启DMA传输
}
