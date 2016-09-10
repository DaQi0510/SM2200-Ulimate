#ifndef __DMA_H
#define __DMA_H
#include "sys.h"	
 	    													  
void dma2_init(void);
void dma1_init(void);
void spi1_tx_dma_transfer_once(void);
void spi1_rx_dma_transfer_once(void);
void spi2_tx_dma_transfer_once(void);
void spi2_rx_dma_transfer_once(void);	

#endif
