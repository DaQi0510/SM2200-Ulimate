#include "Simulation.h"
#include "sm2200.h"
#include "time.h"
#include "led.h"
#include "W5200.h"

extern volatile u8 Device;            //设备号
extern volatile u8 ConnectDevice;     //连接设备号
extern volatile u8 DeviceScale;       //连接从属级别   0从 1主
extern u8 RJ45_2_Connect;             //连接状态
extern u8 RunMode;                    //设备运行模式   轮询模式：1

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
extern u8 RJ45_2_WData[1024];
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
u8 ChannelRece[18];  //  从方记录接收通道数

extern u8 FConnectDevice;  //连接设备号
extern u8 FScale;          //连接设备号从属级别
extern u8 FStartVoltage;   //初始信号幅值
extern u8 FFlag[33];       //标记频点是否被测试过
extern u8 FReceNum[33];    //标记该频点接收次数
extern u8 FNosie[33];      //标记该频点噪声
extern u8 FFrequence[18];  //使用频点值
u8 FindMode;  // 0第一次建立握手  1 对方设备需调整
extern volatile u32 SendTem;
/**
*@brief		通信频点查找频点部分初始化
*@brief   初始频点1,3,5,7,9,11,13,15,16,17,19,21,23,25,27,29,31,33
*/
void FrequenceInit(void)
{
	u8 i,j;
	for(i=0;i<18;i++)     //通信频点查找初始信号频点配置
	{
		if(i<8)
			FFrequence[i]=2*i+1;
		if(i==8)
			FFrequence[i]=16;
		if(i>8)
			FFrequence[i]=2*i-1;	
	}
//	for(i=1;i<34;i++)  //标价通道是否经过测试 1测试过 0没测试过
//	{
//		FFlag[i-1]=0;
//		for(j=0;j<18;j++)
//		{
//			if(FFrequence[j]==i)
//			{
//				FFlag[i-1]=1;
//				break;
//			}
//		}
//	}
	for(i=0;i<18;i++)     //转换成SM2400通信频点
	{
		ChannelFrenquence[i]=FFrequence[j]*3+1;
	}
	SetSm2200Frenquence(18);   //18个通道设置
}
/**
  * 功能：查找通信测试频点，
  * 参数：StartVolatage 起始电压设置
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
u8 FindChatChannel()
{
	u8 i,j,flag;
	if(FScale==1)  //主模式
	{
		FindMode=0;
		flag=MasterCheck_1(FStartVoltage,FConnectDevice);
		if(RJ45_2_Connect==1)     //网络进行监听，返回状态
		{
			RJ45_2_WData[0]=0x3C;   //数据包头
			RJ45_2_WData[1]=0x04;   //应答地址
			RJ45_2_WData[3]=0;      //初始建立握手通道
			RJ45_2_WData[7]=0x3E;   //数据包尾
			RJ45_2_Write(RJ45_2_WData,8);
		}
		if(flag==1)   //握手失败
			return 1;
		SetVoltages();
		MasterCheck_2(FConnectDevice);
	}
	if(FScale==0)  //从模式
	{
		SlaveCheck();
	}
}

/**
  * 功能：第一轮握手频点查找
  * 参数：StartVolatage 起始电压设置
  * 参数：ToDevice 需进行连接的设备号
  * 返回：0 通道接通
 **/
//信息处理  0发送设备地址 1本地址 2电压幅值 3~38通道信息  39-74 信道噪声   75-77发送通道
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice)
{
	u8 i,j;
	Voltage=StartVolatage;
	RJ45_2_WData[5]=0;      //1成功 0 失败
	if(Voltage<MaxVoltage1)    //18个通道
	{
		while(Voltage<MaxVoltage1)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x03ffff;    //18个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}	
			Delayms(100);
		}
		if(Voltage<MaxVoltage1)   //有数据接收到，握手成功
		{		
			RJ45_2_WData[4]=18;     //18个通道模式
			RJ45_2_WData[5]=1;      //1成功 0 失败
			SM2200ReceiveFalg=0;
			ChannelReceive=0; 
			ChannelSend=0;
			return 0;
		}
	}
	
	if((Voltage>=MaxVoltage1)&&(Voltage<MaxVoltage2)) //9个通道
	{
		while(Voltage<MaxVoltage2)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x015555;    //9个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
			Delayms(100);			
		}
		if(Voltage<MaxVoltage2)    
		{
			RJ45_2_WData[5]=1;      //1成功 0 失败
			RJ45_2_WData[4]=9;     //9个通道模式
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else              //更换另外9个通道，以最大电压
		{
			SendTem =0x02AAAA;    //9个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1成功 0 失败
				RJ45_2_WData[4]=9;     //9个通道模式
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
		}
	}
	
	if((Voltage>=MaxVoltage2)&&(Voltage<MaxVoltage3)) //6个通道
	{
		while(Voltage<MaxVoltage3)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x9249;    //6个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
			Delayms(100);
		}
		if(Voltage<MaxVoltage3)
		{
			RJ45_2_WData[5]=1;      //1成功 0 失败
			RJ45_2_WData[4]=6;     //6个通道模式
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else
		{
			SendTem =0x12492;    //6个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建	
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1成功 0 失败
				RJ45_2_WData[4]=6;     //9个通道模式
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
			Delayms(100);
		}
		if(Voltage==MaxVoltage3)   //6个通道发送
		{
			SendTem = SendTem<<1;    //6个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建	
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1成功 0 失败
				RJ45_2_WData[4]=6;     //9个通道模式
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
			Delayms(100);
		}
	}
	if((Voltage>=MaxVoltage3)&&(Voltage<MaxVoltage4)) //3个通道
	{
		while(Voltage<MaxVoltage4)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x1041;    //6个通道发送
			ChannelSend=SendTem;
			MSetInformation();    //数据包组建
			SM2200_Send();        //数据发送
			SM2200ReceiveFalg=0;
			MWaitAck();  //等待应答
			if(SM2200ReceiveFalg!=0)
			{
				break;
			}
			else
			{
				Voltage++;
			}
			Delayms(100);
		}
		if(Voltage<MaxVoltage4)
		{
			RJ45_2_WData[5]=1;      //1成功 0 失败
			RJ45_2_WData[4]=3;     //18个通道模式
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else
		{
			for(i=0;i<5;i++)
			{
				SendTem = SendTem<<1;    //6个通道发送
				ChannelSend=SendTem;
				MSetInformation();    //数据包组建	
				SM2200_Send();        //数据发送
				SM2200ReceiveFalg=0;
				MWaitAck();  //等待应答
				if(SM2200ReceiveFalg!=0)
				{
					RJ45_2_WData[5]=1;      //1成功 0 失败
					RJ45_2_WData[4]=3;     //9个通道模式
					SM2200ReceiveFalg=0;
					ChannelReceive=0;  
					return 0;
				}
				Delayms(100);
			}
		}
	}	
	return 1;  //频点不通
}
//设定最终设定电压
void SetVoltages(void)
{
	if(Voltage<=MaxVoltage1)     //18个通道
	{
		if(MaxVoltage1-Voltage>=5)
		{
			Voltage +=5;
		}
		else
		{
			Voltage=MaxVoltage1;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
	}
	if((Voltage<=MaxVoltage2)&&(Voltage>MaxVoltage1))    //9个通道
	{
		if(MaxVoltage2-Voltage>=5)
		{
			Voltage +=5;
		}
		else
		{
			Voltage=MaxVoltage2;
		}
	}
	if((Voltage<=MaxVoltage3)&&(Voltage>MaxVoltage2))  //6个通道
	{
		if(MaxVoltage3-Voltage>=5)
		{
			Voltage +=5;
		}
		else
		{
			Voltage=MaxVoltage3;
		}
	}
	if((Voltage<=MaxVoltage4)&&(Voltage>MaxVoltage3))   //3个通道
	{
		if(MaxVoltage4-Voltage>=5)
		{
			Voltage +=5;
		}
		else
		{
			Voltage=MaxVoltage4;
		}
		OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
	}
}
/**
  * 功能：第二轮握手频点查找，确定握手频点
  * 返回：0 通道接通
 **/
u8 MasterCheck_2(u8 ToDevice)
{
	u8 i,j;
	for(j=0;j<18;j++)
	{
		if(SendTem&(1<<j))   //该通道已验证
			FFlag[(ChannelFrenquence[j]-1)/3]=1;
	}
	for(i=0;i<3;i++)
	{
		ChannelSend=SendTem;
		MSetInformation();    //数据包组建
		SM2200_Send();        //数据发送
		SM2200ReceiveFalg=0;
		MWaitAck();  //等待应答
		if(ChannelReceive!=0)     //数据包处理
		{
			for(j=0;j<18;j++)
			{
				if(ChannelReceive&(1<<j))   //该通道有值
					FReceNum[(ChannelFrenquence[j]-1)/3]+=1;
			}
		}
		ChannelReceive=0;
		Delayms(80);     //延时80ms
	}
}


/**
  * 功能：第二轮握手返回信息处理
  * 返回：0 执行成功
 **/
u8 DealInformation_1(void)
{
	u8 i,j,k;
	for(i=0;i<18;i++)
	{
		
	}
//	MReceiveChannelNum=0;     
//	SReceiveChannelNum =0;
//	for(i=0;i<18;i++)                  //18个通道，收集导通信息
//	{
//		if((ChannelReceive)&(1<<i))      //该通道有值
//		{
//			
//			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
//			MChannelNoise[MReceiveChannelNum]=OfdmXcvrRead(CARRIER_NOISE,2);
//			MChannelFrequence[MReceiveChannelNum]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
//			MReceiveChannelNum++;
//			if(SReceiveChannelNum==0)
//			{
//				for(j=0;j<18;j++)
//				{
//					if(SM2200RxBuf[i][j*2+2]!=0)
//					{
//						SChannelFrequence[SReceiveChannelNum]=SM2200RxBuf[i][j*2+2];
//						SChannelNoise[SReceiveChannelNum]=SM2200RxBuf[i][j*2+3];
//						SReceiveChannelNum++;	
//					}
//				}
//			}
//		}
//	}
//	//按照噪声值排序,主动方
//	for(i=0;i<MReceiveChannelNum;i++)
//	{
//		k=0;
//		for(j=0;j<MReceiveChannelNum;j++)
//		{
//			if(i!=j)
//			{
//				if(MChannelNoise[i]>MChannelNoise[j])
//					k++;
//				if(MChannelNoise[i]==MChannelNoise[j])
//				{
//					if(MChannelFrequence[i]<MChannelFrequence[j])
//						k++;
//				}
//			}	
//		}
//		Index[i]=k;
//	}
//	for(i=0;i<MReceiveChannelNum;i++)
//	{
//		RelayNosie[Index[i]]=MChannelNoise[i];            //噪声中继
//		RelayFrequence[Index[i]]=MChannelFrequence[i];        //频点中继
//	}
//	for(i=0;i<MReceiveChannelNum;i++)
//	{
//		MChannelNoise[i]=RelayNosie[i];
//		MChannelFrequence[i]=RelayFrequence[i];
//	}
//	//按照噪声值排序,接收方
//	for(i=0;i<SReceiveChannelNum;i++)
//	{
//		k=0;
//		for(j=0;j<SReceiveChannelNum;j++)
//		{
//			if(i!=j)
//			{
//				if(SChannelNoise[i]>SChannelNoise[j])
//					k++;
//				if(SChannelNoise[i]==SChannelNoise[j])
//				{
//					if(SChannelFrequence[i]<SChannelFrequence[j])
//						k++;
//				}
//			}	
//		}
//		Index[i]=k;
//	}
//	
//	for(i=0;i<SReceiveChannelNum;i++)
//	{
//		RelayNosie[Index[i]]=SChannelNoise[i];            //噪声中继
//		RelayFrequence[Index[i]]=SChannelFrequence[i];        //频点中继
//	}
//	for(i=0;i<SReceiveChannelNum;i++)
//	{
//		SChannelNoise[i]=RelayNosie[i];
//		SChannelFrequence[i]=RelayFrequence[i];
//	}
//	ChannelNum_1=0;
//	//通信频点设置
//	if(SReceiveChannelNum<M_SChannelNum)      //主对从的通道设置
//	{
//		for(i=0;i<SReceiveChannelNum;i++)
//			ChannelFrenquence[i]=SChannelFrequence[i];
//		ChannelNum_1=SReceiveChannelNum;
//	}
//	else
//	{
//		for(i=0;i<M_SChannelNum;i++)
//			ChannelFrenquence[i]=SChannelFrequence[i];
//		ChannelNum_1=M_SChannelNum;	
//	}
//	if(MReceiveChannelNum<S_MChannelNum)      //从对主的通道设置
//	{
//		for(i=0;i<MReceiveChannelNum;i++)
//		{
//			for(j=0;j<ChannelNum_1;j++)
//			{
//				if(MChannelFrequence[i]==ChannelFrenquence[j])
//				{
//					break;
//				}
//			}
//			if(j==ChannelNum_1)
//			{
//				ChannelFrenquence[ChannelNum_1]=MChannelFrequence[i];
//				ChannelNum_1++;
//			}
//		}	
//	}
//	else
//	{
//		for(i=0;i<S_MChannelNum;i++)
//		{
//			for(j=0;j<ChannelNum_1;j++)
//			{
//				if(MChannelFrequence[i]==ChannelFrenquence[j])
//				{
//					break;
//				}
//			}
//			if(j==ChannelNum_1)
//			{
//				ChannelFrenquence[ChannelNum_1]=MChannelFrequence[i];
//				ChannelNum_1++;
//			}
//		}	
//	}
//	SM2200ReceiveFalg=0;
//  ChannelReceive=0;
//	return 0;
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
  * 功能：主设备进行通信频点测试时进行应答
 **/
void SlaveCheck(void)
{
	u8 i,j;
	u16 k;
	while(SM2200ReceiveFalg==0)
	{
		if(RunMode!=3) //运行模式改变，跳出循环
			break;
	}
	if(SM2200ReceiveFalg!=0)  //有数据
	{
		for(i=0;i<18;i++)
		{
			if(ChannelReceive&1<<i)  //找到数据包并进行解析
			{
				ConnectDevice=SM2200RxBuf[i][1];
				Voltage =SM2200RxBuf[i][2];
				OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
				FindMode =SM2200RxBuf[i][3];
				if(FindMode==1)   //需完成通信频点设置、
				{
					for(j=0;j<18;j++)
						ChannelFrenquence[j]=SM2200RxBuf[i][4+2*j];
					SetSm2200Frenquence(18);   //18个通道设置
				}
				ChannelSend=(SM2200RxBuf[i][58]<<16)+(SM2200RxBuf[i][59]<<8)+SM2200RxBuf[i][60];
				break;
			}
		}
		ReadRoise();
		SSetInformation();
		SM2200_Send();        //数据发送
	}
}
/**
  *功能：主设备组建载波发送数据包
  *格式：//通信频点组包4-39，40-57信道噪声组包 58 59 60 发送通道组包  
 **/
void MSetInformation(void)
{
	u8 i,j;
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][0]=FConnectDevice;
		SM2200TxBuf[i][1]=Device;              //设备号
	  SM2200TxBuf[i][2]=Voltage;             //信号强度
		SM2200TxBuf[i][3]=FindMode ;           //运行方式
		for(j=0;j<18;j++)                      //通信频点组包4-39，40-57信道噪声组包 58 59 60 发送通道组包                     
		{
			SM2200TxBuf[i][4+j*2]=ChannelFrenquence[j];
			SM2200TxBuf[i][5+j*2]=0;
		}	                              //频点信息	
		ChannelSize[i]=78;              //数据长度          
		ChannelType[i]=0;              //传输类型
			
		SM2200TxBuf[i][58]=ChannelSend/65536;
		SM2200TxBuf[i][59]=(ChannelSend%65536)/256;
		SM2200TxBuf[i][60]=ChannelSend%256;	  			
	}
}

/**
  *功能：主设备等待从设备应答
 **/
void MWaitAck(void)
{
	TIM3->CNT=0;
	while(TIM3->CNT<5000)    //延时500ms
	{
		if(SM2200ReceiveFalg!=0)
			break;
	}
	TIM3->CNT=0;
	while(TIM3->CNT<500);   //延时50ms
}
/**
  *功能：延时函数
 **/
void Delayms(u16 ms)
{
	TIM3->CNT=0;
	ms=ms*10;
	while(TIM3->CNT<ms)    //延时500ms
	{	
	}
}
/**
  *功能：读取信道噪声,并且读取接收到的信道数
 **/
void ReadRoise(void)	
{
	u8 i;
	u16 j;
	for(i=0;i<18;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
		j=OfdmXcvrRead(CARRIER_NOISE,2);
		if(j>255)
			j=255;
		SChannelNoise[i]=j;
		if(ChannelReceive&1<<i)
			ChannelRece[i]=1;
		else
			ChannelRece[i]=0;
	}
}
/**
  *功能：从设备组建载波发送数据包
  *格式：//通信频点组包4-39，40-57信道噪声组包 58 59 60 发送通道组包  
 **/
void SSetInformation(void)
{
	u8 i,j;
	//数据包组建
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][0]=ConnectDevice;
		SM2200TxBuf[i][1]=Device;              //设备号
		SM2200TxBuf[i][2]=Voltage;             //信号强度
		SM2200TxBuf[i][3]=FindMode ;           //信号强度
		for(j=0;j<18;j++)                      //通信频点组包4-39，40-57信道噪声组包 58 59 60 发送通道组包                     
		{
			SM2200TxBuf[i][4+j*2]=ChannelFrenquence[j];
			SM2200TxBuf[i][5+j*2]=ChannelRece[j];
			SM2200TxBuf[i][40+j]=SChannelNoise[j];
		}
		SM2200TxBuf[i][58]=ChannelSend/65536;
		SM2200TxBuf[i][59]=(ChannelSend%65536)/256;
		SM2200TxBuf[i][60]=ChannelSend%256;	 
		ChannelSize[i]=78;              //数据长度          
		ChannelType[i]=0;              //传输类型		
	}
}

