#ifndef __ETHERNET_H
#define __ETHERNET_H															    
#include "sys.h"
#include "w5300.h"

void ethernet_config(void);
void w5300_write(unsigned char *pbuffer,unsigned int add,unsigned char n);
void w5300_read(unsigned char *pbuffer,unsigned int add,unsigned char n);
void ethernet_init(void);
void tcp_sever(void);
void tcp_client(void);
unsigned char is_con(void);
unsigned long recv_len(void);
unsigned char is_send(void);
void recv_data(unsigned char *pbuffer,unsigned long pack_size);
void send_data(unsigned char *pbuffer,unsigned long send_size);

#endif
