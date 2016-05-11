#ifndef __W5200_H
#define __W5200_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_spi.h"

typedef struct _CONFIG_MSG
{
	u8 Mac[6];
	u8 Lip[4];
	u8 Sub[4];
	u8 Gw[4];
	u8 DNS_Server_IP[4];	
	u8  DHCP;
}
CONFIG_MSG;

#define RJ45_1_RST_High  GPIO_SetBits(GPIOE,GPIO_Pin_12)
#define RJ45_1_RST_Low   GPIO_ResetBits(GPIOE,GPIO_Pin_12)
#define RJ45_2_RST_High  GPIO_SetBits(GPIOA,GPIO_Pin_2)
#define RJ45_2_RST_Low   GPIO_ResetBits(GPIOA,GPIO_Pin_2)

#define RJ45_1_CS_High   GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define RJ45_1_CS_Low    GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define RJ45_2_CS_High   GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define RJ45_2_CS_Low    GPIO_ResetBits(GPIOA,GPIO_Pin_4)

#define Success 1
#define Fail    0

#define Ok      1
#define Busy    0

#define Sn_RX_Mask  0x1FFF     //大小为8K
#define Sn_TX_Mask  0x1FFF     //大小为8K

#define Sn_RX_Base  0xC000  
#define Sn_TX_Base  0x8000 


#define TCP_MODE 0x01
//Sn_CR命令部分
#define OPEN     0x01
#define LISTEN   0x02
#define CONNECT  0x04
#define DISCON   0x08
#define CLOSE    0x10
#define SEND     0x20
#define RECV     0x40
#define SEND_KEEP   0x22
//Sn_IR各位部分
#define SEND_OK  0x10
#define TIMEOUT  0x08
#define RECEV    0x04
#define DISCON_Flag   0x02
#define CON      0x01
//Sn_SR各位部分
#define SOCK_CLOSED  0x00
#define SOCK_INIT    0x13
#define SOCK_LISTEN  0x14
#define SOCK_SYNSENT 0x15
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

//相关寄存器设置
#define GAR    0x0001   //GateWay
#define SUBR   0x0005   //Subnet Mask
#define SHAR   0x0009   //MAC
#define SIPR   0x000F   //Local IP
#define RTR    0x0017   //重复计时器
#define RCR    0x0019   //重发计数器
#define W5200_IR     0x0034   //端口中断寄存器
#define PHYSTATUS    0x0035   //PHY状态寄存器，BIT5=0，连接断开；BIT5=1，建立连接；BIT3=0，正常运行模式，BIT3=1，低功耗模式
#define W5200_IMR    0x0016   //中断屏蔽寄存器

#define Sn_MR         0x4000    //Socket Mode register
#define Sn_CR         0x4001    //Socket Command register
#define Sn_IR         0x4002    //Socket Interrupt register
#define Sn_SR         0x4003    //Socket State register
#define Sn_PORT       0x4004    //Source port register
#define Sn_DIPR       0x400C    //目的设备IP号
#define Sn_DPORT      0x4010    //目的设备端口号
#define Sn_RXMEM_SIZE 0x401E    //Receive memory size reigster
#define Sn_TXMEM_SIZE 0x401F    //Transmit memory size reigster
#define Sn_IMR        0x402C    //中断屏蔽寄存器
#define Sn_TX_FSR     0x4020    //发送剩余空间寄存器 
#define Sn_TX_RD      0x4022   
#define Sn_TX_WR      0x4024  
#define Sn_Rx_RSR     0x4026    //接收数据字节大小寄存器
#define Sn_RX_RD      0x4028    //接收数据始地址
#define Sn_RX_WR      0x402A    //接收数据始地址


void RJ45_1_GPIO_Init(void);
void RJ45_1_Reset(void);
void RJ45_1_Init(void);
u8 RJ45_1_TCP_ClientInit(void);
u8 RJ45_1_TCP_ServiceInit(void);
void RJ45_1_TcpOpen(void);
void RJ45_1_Read(u8 *RData);
void RJ45_1_Write(u8 *WData,u16 Len);
void RJ45_1_Write_Register(u16 Address,u8 Data);
u8 RJ45_1_Read_Register(u16 Address);
u8 RJ45_1_ReadWriteByte(u8 TxData);

void RJ45_2_GPIO_Init(void);
void RJ45_2_Reset(void);
void RJ45_2_Init(void);
u8 RJ45_2_TCP_Init(void);
void RJ45_2_Read(u8 *RData);
void RJ45_2_Write(u8 *WData,u16 Len);
void RJ45_2_Write_Register(u16 Address,u8 Data);
u8 RJ45_2_Read_Register(u16 Address);
u8 RJ45_2_ReadWriteByte(u8 TxData);
void RJ45_2_Deal(void); //接收数据处理
	
void ShakeHand(void);
#endif


