#ifndef __Simulation_H
#define __Simulation_H

#include "sys.h"

void FrequenceInit(void);   //∆µ¬ …Ë÷√
u8 FindChatChannel();
u8 Simulation(u8 Device1,u8 Device2);
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice);
void SetVoltages(void);
u8 MasterCheck_2(u8 ToDevice);
u8 DealInformation_1(void);
void SlaveCheck(void);
void MSetInformation(void);
void MWaitAck(void);
void ReadRoise(void);
void SSetInformation(void);
void Delayms(u16 ms);

#endif
