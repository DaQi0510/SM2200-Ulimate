#ifndef __Simulation_H
#define __Simulation_H

#include "sys.h"

u8 FindChatChannel();
u8 Simulation(u8 Device1,u8 Device2);
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice);
u8 MasterCheck_2(u8 ToDevice);
u8 DealInformation_1(void);
u8 SlaveCheck_1(u8 Todevice);
u8 SlaveCheck_2(u8 Todevice);

#endif
