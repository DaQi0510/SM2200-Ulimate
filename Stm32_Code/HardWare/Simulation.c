#include "Simulation.h"
#include "sm2200.h"
#include "time.h"
#include "led.h"
#include "W5200.h"

extern volatile u8 Device;            //设备号
extern volatile u8 ConnectDevice;     //连接设备号
extern volatile u8 DeviceScale;       //连接从属级别   0从 1主

extern volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 ChannelSize[18];         //各通道数据长度
extern volatile u8 ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记
extern volatile u16 Noise[18];              //记录通道噪声
extern volatile u8  ShakeChannel[18];       //记录通道噪声
extern u8 RJ45_1_WData[1024];
extern u8 Voltage;                      //记录电压幅值

/****************第一轮发现通信通道相关变量*********************/
u8 volatile MChannelFrequence[18];     //发送方接收到频点数组 
u8 volatile SChannelFrequence[18];     //接收方接收到频点数组 
u8 volatile MChannelNoise[18];         //发送方接收到频点噪声
u8 volatile SChannelNoise[18];         //接收方接收到频点噪声
u8 volatile SReceiveChannelNum;        //接收方接收到通道数
u8 volatile MReceiveChannelNum;        //发送方接收到通道数
u8 volatile Index[18];                 //记录排序标号
u8 volatile RelayNosie[18];            //噪声中继
u8 volatile RelayFrequence[18];        //频点中继
u8 volatile M_SChannelNum=2;           //发送方到接收方用于通信的通道数目
u8 volatile S_MChannelNum=2;           //接收方到发送方用于通信的通道数目
u8 volatile ChannelNum_1;              //发送方和接收方握手占用总共通道数

/**
  * 功能：查找通信测试频点，
  * 参数：StartVolatage 起始电压设置
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 FindChatChannel()
{
	u8 i,j;
	if(Device==1)    //主动发送方
	{
		if(MasterCheck_1(MinVoltage1,4)==1)    //第一次未能找到通信通道
		{
			RJ45_1_WData[0]='F';
			RJ45_1_WData[1]='a';
			RJ45_1_WData[3]='i';
			RJ45_1_WData[4]='l';
			RJ45_1_WData[5]=1;
			RJ45_1_Write(RJ45_1_WData,128);
			return 1;
		}	
     		
		else                                            //第一次发现通信通道
		{
			RJ45_1_WData[0]='O';
			RJ45_1_WData[1]='k';
			RJ45_1_WData[3]=0;
			RJ45_1_Write(RJ45_1_WData,128);
		}
		delay_ms(100);
		if(MasterCheck_2(4)==1)            //第二次未能找到通信通道     
		{
			RJ45_1_WData[0]='F';
			RJ45_1_WData[1]='a';
			RJ45_1_WData[3]='i';
			RJ45_1_WData[4]='l';
			RJ45_1_WData[5]=2;
			RJ45_1_Write(RJ45_1_WData,128);
			return 1;
		}
		else
		{
			DealInformation_1();
		}
		delay_ms(5000);
	}
	if(Device==4)
	{
		 u8 FrequenceInformation[18];	
//第一圈接收信息
		while(SM2200ReceiveFalg==0);
		TIM3->CNT=0;
		while(TIM3->CNT<300);
    //返回信息，0~1地址 2频点 3噪声 4~6接收通道值  
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				Voltage=SM2200RxBuf[j][38];	
			  OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //通道选择
				Noise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
				RJ45_1_WData[3*j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
				FrequenceInformation[j]=RJ45_1_WData[3*j];
//				RJ45_1_WData[3*j+1]=Noise[j];
//				RJ45_1_WData[3*j+2]=Voltage;	
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];			
			}				
//			else
//			{
//				RJ45_1_WData[3*j]=0;
//				RJ45_1_WData[3*j+1]=0;
//				RJ45_1_WData[3*j+2]=0;
//			}
		}
    //18通道发送信息		
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				SM2200TxBuf[j][0]=0;
				SM2200TxBuf[j][1]=1;
				for(i=0;i<18;i++)
				{
					SM2200TxBuf[i][j*2+2]=FrequenceInformation[j];
					SM2200TxBuf[i][j*2+3]=Noise[j];
				}
				SM2200TxBuf[i][39]=ChannelReceive/65536;
				SM2200TxBuf[i][40]=(ChannelReceive%65536)/256;
				SM2200TxBuf[i][41]=ChannelReceive%256;
			}
			else
			{
				for(i=0;i<18;i++)
				{
					SM2200TxBuf[i][j*2+2]=0;
					SM2200TxBuf[i][j*2+3]=0;
				}
			}
		}
		for(i=0;i<18;i++)
		{
			SM2200TxBuf[i][38]=Voltage;
			ChannelSize[i]=45;
	    ChannelType[i]=0;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);	
//		if(RJ45_1_TcpSetFlag==1)
//		{
//			RJ45_1_WData[127]=1;
////			RJ45_1_Write(RJ45_1_WData,128);
//		}
		ChannelSend =ChannelReceive;
		SM2200_Send();
		ChannelReceive=0;	
		SM2200ReceiveFalg=0;
//第一圈接收信息
		
//第二圈接收信息
		LED1=1;
//	}
		while(SM2200ReceiveFalg==0);
		TIM3->CNT=0;
		while(TIM3->CNT<300);
    //返回信息，0~1地址 2频点 3噪声 4~6接收通道值  
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				Voltage=SM2200RxBuf[j][38];	
			  OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //通道选择
				Noise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
				RJ45_1_WData[3*j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
				FrequenceInformation[j]=RJ45_1_WData[3*j];
//				RJ45_1_WData[3*j+1]=Noise[j];
//				RJ45_1_WData[3*j+2]=Voltage;	
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];			
			}				
//			else
//			{
//				RJ45_1_WData[3*j]=0;
//				RJ45_1_WData[3*j+1]=0;
//				RJ45_1_WData[3*j+2]=0;
//			}
		}
    //18通道发送信息		
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				SM2200TxBuf[j][0]=0;
				SM2200TxBuf[j][1]=1;
				for(i=0;i<18;i++)
				{
					SM2200TxBuf[i][j*2+2]=FrequenceInformation[j];
					SM2200TxBuf[i][j*2+3]=Noise[j];
				}
				SM2200TxBuf[i][39]=ChannelReceive/65536;
				SM2200TxBuf[i][40]=(ChannelReceive%65536)/256;
				SM2200TxBuf[i][41]=ChannelReceive%256;
			}
			else
			{
				for(i=0;i<18;i++)
				{
					SM2200TxBuf[i][j*2+2]=0;
					SM2200TxBuf[i][j*2+3]=0;
				}
			}
		}
		for(i=0;i<18;i++)
		{
			SM2200TxBuf[i][38]=Voltage;
			ChannelSize[i]=45;
	    ChannelType[i]=0;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);	
//		if(RJ45_1_TcpSetFlag==1)
//		{
//			RJ45_1_WData[127]=1;
////			RJ45_1_Write(RJ45_1_WData,128);
//		}
		ChannelSend =ChannelReceive;
		SM2200_Send();
		ChannelReceive=0;	
		SM2200ReceiveFalg=0;
//第一圈接收信息
		
//第二圈接收信息
		LED1=1;
	}
}

/**
  * 功能：第一轮握手频点查找
  * 参数：StartVolatage 起始电压设置
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
//信息处理  0~1发送设备地址   2~37通道信息  38 电压幅值  39~41发送通道
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice)
{
	u8 i,j;
	Voltage=StartVolatage;
	if(Voltage<MaxVoltage1)    //18个通道
	{
		while(Voltage<MaxVoltage1)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=0;i<18;i++)
			{				
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //设备号
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //频点信息
				SM2200TxBuf[i][38]=Voltage;     //电压幅值		
				ChannelSize[i]=45;              //数据长度          
				ChannelType [i]=0;              //传输类型
				ChannelSend|=1<<i;              //发送通道数
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			SM2200ReceiveFalg=0;
			while(TIM3->CNT<6000)    //延时600ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //延时50ms
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}	
		}
		if(Voltage<MaxVoltage1)
		{
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
	}
	
	if((Voltage>=MaxVoltage1)&&(Voltage<MaxVoltage2)) //9个通道
	{
		while(Voltage<MaxVoltage2)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=1;i<18;i=i+2)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //设备号
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //频点信息
				SM2200TxBuf[i][38]=Voltage;     //电压幅值		
				ChannelSize[i]=45;              //数据长度          
				ChannelType [i]=0;              //传输类型
				ChannelSend|=1<<i;              //发送通道数
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //延时500ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //延时20ms
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
		}
		if(Voltage<MaxVoltage2)
		{
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
	}
	
	if((Voltage>=MaxVoltage2)&&(Voltage<MaxVoltage3)) //6个通道
	{
		while(Voltage<MaxVoltage3)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=2;i<18;i=i+3)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //设备号
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //频点信息
				SM2200TxBuf[i][38]=Voltage;     //电压幅值		
				ChannelSize[i]=45;              //数据长度          
				ChannelType[i]=0;              //传输类型
				ChannelSend|=1<<i;              //发送通道数
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //延时500ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //延时20ms
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
		}
		if(Voltage<MaxVoltage3)
		{
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
	}
	if((Voltage>=MaxVoltage3)&&(Voltage<MaxVoltage4)) //3个通道
	{
		while(Voltage<MaxVoltage4)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=4;i<18;i=i+6)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //设备号
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //频点信息
				SM2200TxBuf[i][38]=Voltage;     //电压幅值		
				ChannelSize[i]=45;              //数据长度          
				ChannelType[i]=0;              //传输类型
				ChannelSend|=1<<i;              //发送通道数
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //延时600ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //延时20ms
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
		}
		if(Voltage<MaxVoltage4)
		{
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
	}	
	return 1;  //频点不通
}

/**
  * 功能：第二轮握手频点查找，在第一轮基础上加3
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 MasterCheck_2(u8 ToDevice)
{
	u8 i,j;
	if(Voltage<=MaxVoltage1)     //18个通道
	{
		if(MaxVoltage1-Voltage>=3)
		{
			Voltage +=3;
		}
		else
		{
			Voltage=MaxVoltage1;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
		ChannelSend=0;
		for(i=0;i<18;i++)
		{				
			SM2200TxBuf[i][0]=ToDevice/256;
			SM2200TxBuf[i][1]=ToDevice%256;             //设备号
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //频点信息
			SM2200TxBuf[i][38]=Voltage;     //电压幅值		
			ChannelSize[i]=45;              //数据长度          
			ChannelType[i]=0;               //传输类型
			ChannelSend|=1<<i;              //发送通道数
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //延时500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //延时20ms
	}
	
	if((Voltage<=MaxVoltage2)&&(Voltage>MaxVoltage1))    //9个通道
	{
		if(MaxVoltage2-Voltage>=3)
		{
			Voltage +=3;
		}
		else
		{
			Voltage=MaxVoltage2;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
		ChannelSend=0;
		for(i=1;i<18;i=i+2)
		{
			SM2200TxBuf[i][0]=ToDevice/256;
			SM2200TxBuf[i][1]=ToDevice%256;             //设备号
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //频点信息
			SM2200TxBuf[i][38]=Voltage;     //电压幅值		
			ChannelSize[i]=45;              //数据长度          
			ChannelType[i]=0;               //传输类型
			ChannelSend|=1<<i;              //发送通道数
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<5000)    //延时500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<200);   //延时20ms
	}
	
	if((Voltage<=MaxVoltage3)&&(Voltage>MaxVoltage2))  //6个通道
	{
		if(MaxVoltage3-Voltage>=3)
		{
			Voltage +=3;
		}
		else
		{
			Voltage=MaxVoltage3;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
		ChannelSend=0;
		for(i=2;i<18;i=i+3)
		{
			SM2200TxBuf[i][0]=ToDevice/256;
			SM2200TxBuf[i][1]=ToDevice%256;             //设备号
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //频点信息
			SM2200TxBuf[i][38]=Voltage;     //电压幅值		
			ChannelSize[i]=45;              //数据长度          
			ChannelType[i]=0;               //传输类型
			ChannelSend|=1<<i;              //发送通道数
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //延时500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //延时20ms
	}
	
	if((Voltage<=MaxVoltage4)&&(Voltage>MaxVoltage3))   //3个通道
	{
		if(MaxVoltage4-Voltage>=3)
		{
			Voltage +=3;
		}
		else
		{
			Voltage=MaxVoltage4;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
		ChannelSend=0;
		for(i=4;i<18;i=i+6)
		{
			SM2200TxBuf[i][0]=ToDevice/256;
			SM2200TxBuf[i][1]=ToDevice%256;             //设备号
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //频点信息
			SM2200TxBuf[i][38]=Voltage;     //电压幅值		
			ChannelSize[i]=45;              //数据长度          
			ChannelType[i]=0;               //传输类型
			ChannelSend|=1<<i;              //发送通道数
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //延时500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //延时20ms
	}
	
	if(SM2200ReceiveFalg==0)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}


/**
  * 功能：第二轮握手返回信息处理
  * 返回：0 执行成功
 **/
u8 DealInformation_1(void)
{
	u8 i,j,k;
	MReceiveChannelNum=0;     
	SReceiveChannelNum =0;
	for(i=0;i<18;i++)                  //18个通道，收集导通信息
	{
		if((ChannelReceive)&(1<<i))      //该通道有值
		{
			
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
			MChannelNoise[MReceiveChannelNum]=OfdmXcvrRead(CARRIER_NOISE,2);
			MChannelFrequence[MReceiveChannelNum]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
			MReceiveChannelNum++;
			if(SReceiveChannelNum==0)
			{
				for(j=0;j<18;j++)
				{
					if(SM2200RxBuf[i][j*2+2]!=0)
					{
						SChannelFrequence[SReceiveChannelNum]=SM2200RxBuf[i][j*2+2];
						SChannelNoise[SReceiveChannelNum]=SM2200RxBuf[i][j*2+3];
						SReceiveChannelNum++;	
					}
				}
			}
		}
	}
	//按照噪声值排序,主动方
	for(i=0;i<MReceiveChannelNum;i++)
	{
		k=0;
		for(j=0;j<MReceiveChannelNum;j++)
		{
			if(i!=j)
			{
				if(MChannelNoise[i]>MChannelNoise[j])
					k++;
				if(MChannelNoise[i]==MChannelNoise[j])
				{
					if(MChannelFrequence[i]<MChannelFrequence[j])
						k++;
				}
			}	
		}
		Index[i]=k;
	}
	for(i=0;i<MReceiveChannelNum;i++)
	{
		RelayNosie[Index[i]]=MChannelNoise[i];            //噪声中继
		RelayFrequence[Index[i]]=MChannelFrequence[i];        //频点中继
	}
	for(i=0;i<MReceiveChannelNum;i++)
	{
		MChannelNoise[i]=RelayNosie[i];
		MChannelFrequence[i]=RelayFrequence[i];
	}
	//按照噪声值排序,接收方
	for(i=0;i<SReceiveChannelNum;i++)
	{
		k=0;
		for(j=0;j<SReceiveChannelNum;j++)
		{
			if(i!=j)
			{
				if(SChannelNoise[i]>SChannelNoise[j])
					k++;
				if(SChannelNoise[i]==SChannelNoise[j])
				{
					if(SChannelFrequence[i]<SChannelFrequence[j])
						k++;
				}
			}	
		}
		Index[i]=k;
	}
	
	for(i=0;i<SReceiveChannelNum;i++)
	{
		RelayNosie[Index[i]]=SChannelNoise[i];            //噪声中继
		RelayFrequence[Index[i]]=SChannelFrequence[i];        //频点中继
	}
	for(i=0;i<SReceiveChannelNum;i++)
	{
		SChannelNoise[i]=RelayNosie[i];
		SChannelFrequence[i]=RelayFrequence[i];
	}
	ChannelNum_1=0;
	//通信频点设置
	if(SReceiveChannelNum<M_SChannelNum)      //主对从的通道设置
	{
		for(i=0;i<SReceiveChannelNum;i++)
			ChannelFrenquence[i]=SChannelFrequence[i];
		ChannelNum_1=SReceiveChannelNum;
	}
	else
	{
		for(i=0;i<M_SChannelNum;i++)
			ChannelFrenquence[i]=SChannelFrequence[i];
		ChannelNum_1=M_SChannelNum;	
	}
	if(MReceiveChannelNum<S_MChannelNum)      //从对主的通道设置
	{
		for(i=0;i<MReceiveChannelNum;i++)
		{
			for(j=0;j<ChannelNum_1;j++)
			{
				if(MChannelFrequence[i]==ChannelFrenquence[j])
				{
					break;
				}
			}
			if(j==ChannelNum_1)
			{
				ChannelFrenquence[ChannelNum_1]=MChannelFrequence[i];
				ChannelNum_1++;
			}
		}	
	}
	else
	{
		for(i=0;i<S_MChannelNum;i++)
		{
			for(j=0;j<ChannelNum_1;j++)
			{
				if(MChannelFrequence[i]==ChannelFrenquence[j])
				{
					break;
				}
			}
			if(j==ChannelNum_1)
			{
				ChannelFrenquence[ChannelNum_1]=MChannelFrequence[i];
				ChannelNum_1++;
			}
		}	
	}
	SM2200ReceiveFalg=0;
  ChannelReceive=0;
	return 0;
}
/**
  * 功能：第三轮握手确认信息
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 MasterCheck_3(u8 ToDevice)
{
	u8 i,j;
	SetSm2200Frenquence(ChannelNum_1);
	ChannelSend=0;
	for(i=0;i<ChannelNum_1;i++)
	{
		SM2200TxBuf[i][0]=ToDevice/256;
		SM2200TxBuf[i][1]=ToDevice%256;             //设备号
		for(j=0;j<ChannelNum_1;j++)               //频点信息
		{
			SM2200TxBuf[i][j*2+2]=ChannelFrenquence[j];
		}
		SM2200TxBuf[i][38]=Voltage;     //电压幅值		
		ChannelSize[i]=45;              //数据长度          
		ChannelType[i]=0;               //传输类型
		ChannelSend|=1<<i;              //发送通道数
	}
	for(i=0;i<ChannelNum_1;i++)
	{
		SM2200TxBuf[i][39]=ChannelSend/65536;
		SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
		SM2200TxBuf[i][41]=ChannelSend%256;		
	}
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
	SM2200_Send();
	while(SM2200ReceiveFalg==0);
	TIM3->CNT=0;
	while(TIM3->CNT<300);
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			
		}
	}
}

/**
  * 功能：第一轮握手频点查找(从)
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 SlaveCheck_1(u8 Todevice)
{
	u8 i,j;
	while(SM2200ReceiveFalg==0);
	TIM3->CNT=0;
	while(TIM3->CNT<300);
	Voltage=0;
	ChannelSend=0;
	//返回信息，0~1地址 2频点 3噪声 4~6接收通道值  
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			if(Voltage==0)
			{
				Voltage=SM2200RxBuf[j][38];	
			}
			OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //通道选择
			SChannelNoise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
			SChannelFrequence[j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
      if(ChannelSend==0)
			{
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];		
			}				
		}				
	}
	//18通道发送信息		
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			SM2200TxBuf[j][0]=Todevice/256;
			SM2200TxBuf[j][1]=Todevice%256;
			for(i=0;i<18;i++)
			{
				SM2200TxBuf[i][j*2+2]=SChannelFrequence[j];
				SM2200TxBuf[i][j*2+3]=SChannelNoise[j];
			}
			SM2200TxBuf[i][39]=ChannelReceive/65536;
			SM2200TxBuf[i][40]=(ChannelReceive%65536)/256;
			SM2200TxBuf[i][41]=ChannelReceive%256;
		}
		else
		{
			for(i=0;i<18;i++)
			{
				SM2200TxBuf[i][j*2+2]=0;
				SM2200TxBuf[i][j*2+3]=0;
			}
		}
	}
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][38]=Voltage;
		ChannelSize[i]=45;
		ChannelType[i]=0;
	}
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);	
	SM2200_Send();
	ChannelReceive=0;	
	SM2200ReceiveFalg=0;
	return 0;
}
/**
  * 功能：第二轮握手频点查找(从)
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 SlaveCheck_2(u8 Todevice)
{
	u8 i,j;
	while(SM2200ReceiveFalg==0);
	TIM3->CNT=0;
	while(TIM3->CNT<300);
	Voltage=0;
	ChannelSend=0;
	//返回信息，0~1地址 2频点 3噪声 4~6接收通道值  
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			if(Voltage==0)
			{
				Voltage=SM2200RxBuf[j][38];	
			}
			OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //通道选择
			SChannelNoise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
			SChannelFrequence[j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
      if(ChannelSend==0)
			{
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];		
			}				
		}				
	}
	//18通道发送信息		
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			SM2200TxBuf[j][0]=Todevice/256;
			SM2200TxBuf[j][1]=Todevice%256;
			for(i=0;i<18;i++)
			{
				SM2200TxBuf[i][j*2+2]=SChannelFrequence[j];
				SM2200TxBuf[i][j*2+3]=SChannelNoise[j];
			}
			SM2200TxBuf[i][39]=ChannelReceive/65536;
			SM2200TxBuf[i][40]=(ChannelReceive%65536)/256;
			SM2200TxBuf[i][41]=ChannelReceive%256;
		}
		else
		{
			for(i=0;i<18;i++)
			{
				SM2200TxBuf[i][j*2+2]=0;
				SM2200TxBuf[i][j*2+3]=0;
			}
		}
	}
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][38]=Voltage;
		ChannelSize[i]=45;
		ChannelType[i]=0;
	}
	OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);	
	SM2200_Send();
	ChannelReceive=0;	
	SM2200ReceiveFalg=0;
	return 0;
}

