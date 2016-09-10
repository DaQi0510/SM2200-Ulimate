#ifndef __SM2200_H
#define __SM2200_H

#include "delay.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_dma.h"
#include "sys.h"
#include "lcd.h"

#define NUMBER_OF_CLUSTERS  18

#define MinVoltage1 3
#define MaxVoltage1 28    //对应18个通道
#define MinVoltage2 28
#define MaxVoltage2 56    //对应9个通道
#define MinVoltage3 56
#define MaxVoltage3 70    //对应6个通道
#define MinVoltage4 70
#define MaxVoltage4 105    //对应3个通道



#define CLUSTER_SELECT		0x20
#define CARRIER_FREQ_SELECT	0x22
#define TRANCEIVER_CONTROL	0x24
#define SPI_PKTSIZE		0x26
#define BIU_THRESHOLD 		0x28
#define CHANNEL_ENABLE 		0x2A
#define BAND_IN_USE_STAT	0x2C
#define TRANSMIT_STATUS		0x30
#define CARRIER_VOLTAGE		0x32
#define TRANSMIT_BUFFER		0x34
#define SPI_SEND_PKTS		0x36
#define TX_INIT_PHASE		0x38
#define TX_BUFFER_CTRL		0x3A
#define TX_OUT_VOLTAGE		0x3C

#define RECEIVE_STATUS		0x40
#define CARRIER_NOISE		0x42
#define RECEIVE_BUFFER		0x48

#define INTERRUPT_MASK		0x50
#define INTERRUPT_EVENT		0x52
#define SPARE_REG		0x54
#define TRANCEIVER_CONFIG	0x56

#define NODE_ADDR0       	0x60
#define NODE_ADDR1       	0x62
#define NODE_ADDR2       	0x64
#define NODE_ADDR3      	0x66

#define NODE_ADDR_MASK0  	0x68
#define NODE_ADDR_MASK1  	0x6A
#define NODE_ADDR_MASK2  	0x6C
#define NODE_ADDR_MASK3  	0x6E

#define AGC_CONTROL             0x70
#define AGC_LEVELS		0x72
#define TX_SIGNAL_OFFSET_POS	0x74
#define TX_SIGNAL_OFFSET_NEG	0x76

#define PHYS_RESET		0x01
#define LOCAL_LOOPBACK  	0x02
#define INTERLOCK_DISABLE  	0x04
#define RX_ENABLE		0x10
#define BIUQUAL_ENABLE          0x400
#define AGC_ENABLE		0x200
#define SLAVE_SELECT_FRAME 	0x100

#define SPI_NSS_High PDout(0)=1
#define SPI_NSS_Low  PDout(0)=0
#define RST_High     PAout(1)=1
#define RST_Low      PAout(1)=0

void SM2200_Init(void);
void SetSm2200Frenquence(u8 ChannelN);
void SM2200_Send(void);
void SM2200Respond(void);
void SM2200_GPIO(void);
u8 SPI3_ReadWriteByte(u8 TxData);
unsigned long OfdmXcvrRead(unsigned char address, unsigned char size);
void OfdmXcvrWrite(unsigned char address, unsigned char size, unsigned long writeData);
void SPI_SM2200(void);
void SM2200_Receive(void);
void NVIC_EXTI(u8 En);
void Dealyx(u16 num);
#endif



