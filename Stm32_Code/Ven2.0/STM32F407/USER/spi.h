#ifndef __SPI_H
#define __SPI_H
#include "sys.h"	
 	    													  
void spi1_init(void);
void spi2_init(void);
u8 SPI1_ReadWriteByte(u8 TxData);
u8 SPI2_ReadWriteByte(u8 TxData);
		 
#endif
