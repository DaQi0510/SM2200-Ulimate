#ifndef __SRAM_H
#define __SRAM_H															    
#include "sys.h" 

#define Bank1_SRAM3_ADDR    ((u32)(0x68000000))
#define Max_Buffer 250000

void sram_init(void);
void sram_write_byte(u8* pBuffer,u32 WriteAddr,u32 n);
void sram_read_byte(u8* pBuffer,u32 ReadAddr,u32 n);
void sram_write_word(u16* pBuffer,u32 WriteAddr,u32 n);
void sram_read_word(u16* pBuffer,u32 ReadAddr,u32 n);

#endif
