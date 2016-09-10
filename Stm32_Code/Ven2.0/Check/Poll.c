#include "Poll.h"
/**
*@brief	轮询发送函数，主设备轮询发送信息到从设备，从设备等待主设备的信息
*@param Num: 主设备从发次数
*/
extern volatile u32 SendTem;       //临时发送通道
extern volatile u8 DeviceScale;             //连接从属级别   0从 1主
extern volatile u8 ConnectDevice[7];        //连接设备号
extern volatile u8 SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8 SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8 ChannelFrenquence[18];   //18个通道频点数组
extern volatile u8 ChannelSize[18];         //各通道数据长度
extern volatile u8 ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelReceive;         //标记接收的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u8 Device;                  //本机设备号   
extern u8 Voltage;                          //记录电压幅值
extern u8 RunMode;                          //设备运行模式
extern u32 ReceNum[7][19];                  //记录向各个设备的发送、接收次数
extern u16 ReceRoise[7][18];                 //记录各个通道的信号噪声
extern volatile u8 ReDevice;                //接收到数据的设备号
extern u8 RateFlag;                          //通信速率标记位
extern u16 RateTime[7];                      //接收发送一包数据时间
extern u8 Rates[7];                          //接收到通道数
u8 RateTem[7];                               //接收到通道数,临时变量
extern u8 RJ45_2_WData[1024];                
void Poll(u8 Num)
{
	u8 i,j,l,k=0;
	u16 time;
	if(DeviceScale==1)     //主设备
	{
		for(i=0;i<7;i++)
		{
			RateTem[i]=0;
			OneAgain:		  if(RunMode==1)               //判断是否处于轮询模式
			{
				if(ConnectDevice[i]!=0)    //需与该设备建立建立载波通信
				{
					ChannelSend=SendTem;
					/*******发送数据包组建***********/
					for(j=0;j<18;j++)
					{
						SM2200TxBuf[j][0]=ConnectDevice[i];   //地址
						SM2200TxBuf[j][1]=Device;             //本机地址
						SM2200TxBuf[j][2]=Voltage;            //发送信号强度幅值
						for(l=0;l<18;l++)
						{
							SM2200TxBuf[j][3+2*l]=ChannelFrenquence[l];  //发送频点
							if(ChannelSend&(1<<l))
								SM2200TxBuf[j][4+2*l]=1;                  //通道发送使能位
							else
								SM2200TxBuf[j][4+2*l]=0;                  //通道发送使能位
						}
						SM2200TxBuf[j][39]=((ReceNum[i][0]+1)&0xFF000000)>>24;
						SM2200TxBuf[j][40]=((ReceNum[i][0]+1)&0x00FF0000)>>16;
						SM2200TxBuf[j][41]=((ReceNum[i][0]+1)&0x0000FF00)>>8;
						SM2200TxBuf[j][42]=((ReceNum[i][0]+1)&0x000000FF);
						SM2200TxBuf[j][79]=(ChannelSend&0xFF000000)>>24;
						SM2200TxBuf[j][80]=(ChannelSend&0x00FF0000)>>16;
						SM2200TxBuf[j][81]=(ChannelSend&0x0000FF00)>>8;
						SM2200TxBuf[j][82]=(ChannelSend&0x000000FF);
						ChannelSize[j]=88;
						ChannelType[j]=0;
					}
          SM2200_Send();
				  TIM5->CNT=0;   //记录时间
					RateFlag =0;   //清除标志量
		      ChannelSend=0;
          TIM3->CNT=0;                 //清空TIM3计数器			
				  while(SM2200ReceiveFalg==0)	 //等待对方返回应答数据，如果0.5s内无返回，跳出循环
				  {
						time= TIM3->CNT;
						if(time>5000)
							break;
				  }
          if((SM2200ReceiveFalg!=0)&&(RateFlag!=1))	
					{
						RateTime[i]=TIM5->CNT;
					}						
					k++;
          if((SM2200ReceiveFalg==0)&&(k<Num))	
					{
						ReceNum[i][0]+=1;       //发送次数加一
						goto OneAgain;
					}
					ReceNum[i][0]+=1;       //发送次数加一					
          if(SM2200ReceiveFalg!=0)
					{
						TIM3->CNT=0;
						while(ChannelReceive!=0x03ffff)   //等待数据接受完成，30ms后如仍不能接收完成，跳出
						{
							time=TIM3->CNT;
							if(time>300)
								break;
						}
						for(j=0;j<18;j++)                 //数据进行处理，统计
						{
							if(ChannelReceive&1<<j)
							{
								for(l=0;l<18;l++)
								{
									if(SM2200RxBuf[j][4+2*l]==1)
										ReceNum[i][l+1]+=1;     //统计接收到次数
									ReceRoise[i][l]=SM2200RxBuf[j][43+2*l]*256+SM2200RxBuf[j][44+2*l];    //读取信道噪声
									
								}
								break;
							}
						}
						RateTem[i]=0;
						for(j=0;j<18;j++)                //读取接收通道数
						{
							if(ChannelReceive&1<<j)
							{
								RateTem[i]+=1;
							}
						}
						Rates[i]=RateTem[i];
						SM2200ReceiveFalg=0;                //清空计数
						ChannelReceive=0;
					}
					TIM3->CNT=0;
					while(1)
					{
						time= TIM3->CNT;
						if(time>500)
							break;
					}
					k=0;
         	ChannelSend=0;
				}
			}
		}	
	}
	if(DeviceScale==0)    //从设备
	{
		while(SM2200ReceiveFalg==0)   //等待主设备对此发送数据
		{
			if(RunMode!=1)              //模式改变，跳出循环
				break;               
		}
		if(SM2200ReceiveFalg!=0)      //有数据接收到
		{
			TIM3->CNT=0;
			while(ChannelReceive!=0x03ffff)   //等待数据接受完成，30ms后如仍不能接收完成，跳出
			{
				time=TIM3->CNT;
				if(time>300)
					break;
			}
			/*******数据解析和数据包组建***********/
			for(i=0;i<18;i++)
			{
				if(ChannelReceive&(1<<i))
				{
					ReDevice=SM2200RxBuf[i][1];
					if(ReDevice<Device)
					{
						ReceNum[ReDevice-1][0]=(SM2200RxBuf[i][39]<<24)+(SM2200RxBuf[i][40]<<16)+(SM2200RxBuf[i][41]<<8)+SM2200RxBuf[i][42];
						if(ReceNum[ReDevice-1][0]==1)
						{
							for(j=0;j<18;j++)
							{
								ReceNum[ReDevice-1][j+1]=0;
							}
						}
					}
					else
					{
						ReceNum[ReDevice-2][0]=(SM2200RxBuf[i][39]<<24)+(SM2200RxBuf[i][40]<<16)+(SM2200RxBuf[i][41]<<8)+SM2200RxBuf[i][42];
						if(ReceNum[ReDevice-2][0]==1)
						{
							for(j=0;j<18;j++)
							{
								ReceNum[ReDevice-1][j+1]=0;
							}
						}
					}
					Voltage =SM2200RxBuf[i][2];
					OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);//以为着最多可开2~3个通道
					ChannelSend =(SM2200RxBuf[i][79]<<24)+(SM2200RxBuf[i][80]<<16)+(SM2200RxBuf[i][81]<<8)+SM2200RxBuf[i][82];					break;
				}
			}
			//读取信道噪声
			for(i=0;i<18;i++)
			{
				OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //通道选择
				if((ReDevice>0)&&(ReDevice<Device))
				{
					ReceRoise[ReDevice-1][i]=OfdmXcvrRead(CARRIER_NOISE,2);
				}
				if((ReDevice>0)&&(ReDevice>Device))
					ReceRoise[ReDevice-2][i]=OfdmXcvrRead(CARRIER_NOISE,2);
			}
			for(i=0;i<18;i++)
			{
				if(ChannelReceive&(1<<i))
				{
					if(ReDevice<Device)
						ReceNum[ReDevice-1][i+1]+=1;     //统计接收到次数
					else
						ReceNum[ReDevice-2][i+1]+=1;     //统计接收到次数
				}
				//数据包组建
				SM2200TxBuf[i][0]=ReDevice;   //地址
				SM2200TxBuf[i][1]=Device;             //本机地址
				SM2200TxBuf[i][2]=Voltage;            //发送信号强度幅值
				for(l=0;l<18;l++)
				{
					SM2200TxBuf[i][3+2*l]=ChannelFrenquence[l];  //发送频点
					if(ChannelReceive&(1<<l))
						SM2200TxBuf[i][4+2*l]=1;                  //通道发送使能位
					else
						SM2200TxBuf[i][4+2*l]=0;                  //通道发送使能位
				}
				if(ReDevice<Device)
				{
					SM2200TxBuf[i][39]=(ReceNum[ReDevice-1][0]&0xFF000000)>>24;
					SM2200TxBuf[i][40]=(ReceNum[ReDevice-1][0]&0x00FF0000)>>16;
					SM2200TxBuf[i][41]=(ReceNum[ReDevice-1][0]&0x0000FF00)>>8;
					SM2200TxBuf[i][42]=(ReceNum[ReDevice-1][0]&0x000000FF);
					//信道噪声数据包组建
					for(j=0;j<18;j++)
					{
						SM2200TxBuf[i][43+2*j]=(ReceRoise[ReDevice-1][j]&0xFF00)>>8;
						SM2200TxBuf[i][44+2*j]=(ReceRoise[ReDevice-1][j]&0xFF);
					}
				}
				else
				{
					SM2200TxBuf[i][39]=(ReceNum[ReDevice-2][0]&0xFF000000)>>24;
					SM2200TxBuf[i][40]=(ReceNum[ReDevice-2][0]&0x00FF0000)>>16;
					SM2200TxBuf[i][41]=(ReceNum[ReDevice-2][0]&0x0000FF00)>>8;
					SM2200TxBuf[i][42]=(ReceNum[ReDevice-2][0]&0x000000FF);
					for(j=0;j<18;j++)
					{
						SM2200TxBuf[i][43+2*j]=(ReceRoise[ReDevice-2][j]&0xFF00)>>8;
						SM2200TxBuf[i][44+2*j]=(ReceRoise[ReDevice-2][j]&0xFF);
					}
				}
				ChannelSize[i]=88;
				ChannelType[i]=0;
			}
		//	ChannelSend=0x03ffff;
			ChannelReceive=0;
			SM2200ReceiveFalg=0;
			SM2200_Send ();
		  ChannelSend=0;
		}
	}	
}