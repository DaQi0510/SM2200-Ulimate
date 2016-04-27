#ifndef __RJ45_1_H
#define __RJ45_1_H

#define Sn_RX_Mask  0x1FFF     //��СΪ8K
#define Sn_TX_Mask  0x1FFF     //��СΪ8K

#define Sn_RX_Base  0xC000  
#define Sn_TX_Base  0x8000 


#define TCP_MODE 0x01
//Sn_CR�����
#define OPEN     0x01
#define LISTEN   0x02
#define DISCON   0x08
#define CLOSE    0x10
#define SEND     0x20
#define RECV     0x40
#define SEND_KEEP   0x22
//Sn_IR��λ����
#define SEND_OK  0x10
#define TIMEOUT  0x08
#define RECEV    0x04
#define DISCON_Flag   0x02
#define CON      0x01
//Sn_SR��λ����
#define SOCK_CLOSED  0x00
#define SOCK_INIT    0x13
#define SOCK_LISTEN  0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

//��ؼĴ�������
#define GAR    0x0001   //GateWay
#define SUBR   0x0005   //Subnet Mask
#define SHAR   0x0009   //MAC
#define SIPR   0x000F   //Local IP
#define RTR    0x0017   //�ظ���ʱ��
#define RCR    0x0019   //�ط�������
#define W5200_IR     0x0034   //�˿��жϼĴ���
#define PHYSTATUS    0x0035   //PHY״̬�Ĵ�����BIT5=0�����ӶϿ���BIT5=1���������ӣ�BIT3=0����������ģʽ��BIT3=1���͹���ģʽ
#define W5200_IMR    0x0016   //�ж����μĴ���

#define Sn_MR         0x4000    //Socket Mode register
#define Sn_CR         0x4001    //Socket Command register
#define Sn_IR         0x4002    //Socket Interrupt register
#define Sn_SR         0x4003    //Socket State register
#define Sn_PORT       0x4004    //Source port register
#define Sn_RXMEM_SIZE 0x401E    //Receive memory size reigster
#define Sn_TXMEM_SIZE 0x401F    //Transmit memory size reigster
#define Sn_IMR        0x402C    //�ж����μĴ���
#define Sn_TX_FSR     0x4020    //����ʣ��ռ�Ĵ��� 
#define Sn_TX_RD      0x4022   
#define Sn_TX_WR      0x4024  
#define Sn_Rx_RSR     0x4026    //���������ֽڴ�С�Ĵ���
#define Sn_RX_RD      0x4028    //��������ʼ��ַ
#define Sn_RX_WR      0x402A    //��������ʼ��ַ

#endif
