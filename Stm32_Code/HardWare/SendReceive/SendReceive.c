#include "SendReceive.h"
//网口部分
extern u8 Device;          //设备号
extern u8 RJ45_1_RData[1024];
extern u8 RJ45_1_WData[1024];
extern u8 RJ45_1_Connect;  //连接状态
extern u8 RJ45_1_ReceiveFlag;
extern u8 RJ45_2_RData[1024];
extern u8 RJ45_2_WData[1024];
extern u8 RJ45_2_Connect;  //连接状态
extern u8 RJ45_2_ReceiveFlag;//标记网口2是否接收到数据
//SM2200部分
extern volatile u8  SM2200TxBuf[18][128];    //18个通道发送数据包数组
extern volatile u8  SM2200RxBuf[18][128];    //18个通道接收数据包数组
extern volatile u8  ChannelSize[18];         //各通道数据长度
extern volatile u8  ChannelType[18];         //各通道发送模式
extern volatile u32 ChannelSend;            //标记发送的通道
extern volatile u8 SM2200ReceiveFalg;       //当有数据时接收标记
void SendRece(void)
{
	
}
////函数功能:主站通过网口2发送命令，网口1将命令传输到DTU设备
////创建日期：2016年3月30日
//void SendReceive(void)
//{
//	int i,len;
//	while(1)
//	{
//		if(RJ45_1_Connect==1)
//		{
//			if(RJ45_2_ReceiveFlag==1)
//			{
//				RJ45_2_ReceiveFlag=0;
//				len=RJ45_2_RData[0]*256+RJ45_2_RData[1];
//				for(i=0;i<len;i++)
//				{
//					RJ45_1_WData[i]=RJ45_2_RData[i+2];			
//					RJ45_1_Write(RJ45_1_WData,len);
//				}
//			}
//		}
//		if(RJ45_2_Connect==1)
//		{
//			if(RJ45_1_ReceiveFlag==1)
//			{
//				RJ45_1_ReceiveFlag=0;
//				len=RJ45_1_RData[0]*256+RJ45_1_RData[1];
//				for(i=0;i<len;i++)
//				{
//					RJ45_2_WData[i]=RJ45_1_RData[i+2];
//				}
//				RJ45_2_Write(RJ45_2_WData,len);
//			}
//		}
//	}
//}
////函数功能:主站通过网口2发送命令，通过线路将命令传输到DTU设备
////创建日期：2016年3月30日
//void SendReceivec(void)
//{
//	int i,len,j,k;
//	if(Device==4)
//	{
//		if(SM2200ReceiveFalg!=0)
//		{
//			TIM3->CNT=0;
//			while(TIM3->CNT<500);   //延时20ms
//			SM2200ReceiveFalg=0;
//			len=SM2200RxBuf[0][3];
//			if(len<=85)
//			{
//				for(i=0;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//			if((len>85)&&(len<=170))
//			{
//				for(i=0;i<85;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				for(i=85;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[1][i-80];	
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//			if(len>170)
//			{
//				for(i=0;i<85;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				for(i=85;i<170;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[1][i-80];	
//				}
//				for(i=170;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[2][i-165];	
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//		}
//		if(RJ45_1_ReceiveFlag==1)
//		{
//			RJ45_1_ReceiveFlag=0;
//			len=RJ45_1_RData[0]*256+RJ45_1_RData[1];    //读取数据长度
//			SM2200TxBuf[0][0]=0;
//			SM2200TxBuf[0][1]=2;      //地址位
//			SM2200TxBuf[0][2]=len;    //数据包长度
//			if(len<=85)
//			{
//				SM2200TxBuf[0][0]=0;
//			  SM2200TxBuf[0][1]=2;      //地址位
//			  SM2200TxBuf[0][2]=len;    //数据包长度
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=len;  
//				for(i=0;i<len;i++)
//				{
//					SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//				}
//				ChannelSend |=1<<0;
//			  ChannelSize [0]=90;
//			  ChannelType [0]=0;
//			}
//			if((len>85)&&(len<=170))
//			{
//				for(i=0;i<2;i++)
//				{
//					SM2200TxBuf[i][0]=0;
//					SM2200TxBuf[i][1]=2;      //地址位
//					SM2200TxBuf[i][2]=len;    //数据包长度
//				}
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=85;  
//				SM2200TxBuf[1][3]=86;
//				SM2200TxBuf[1][4]=len;
//        for(i=0;i<len;i++)
//				{
//					if(i<85)
//						SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//					else
//						SM2200TxBuf[1][i-80]=RJ45_1_RData[i+2];
//				}
//				for(i=0;i<2;i++)
//				{
//					ChannelSend |=1<<i;
//					ChannelSize [i]=90;
//					ChannelType [i]=0;
//				}	
//			}
//			if(len>170)
//			{
//				for(i=0;i<3;i++)
//				{
//					SM2200TxBuf[i][0]=0;
//					SM2200TxBuf[i][1]=2;      //地址位
//					SM2200TxBuf[i][2]=len;    //数据包长度
//				}
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=85;  
//				SM2200TxBuf[1][3]=86;
//				SM2200TxBuf[1][4]=170;
//				SM2200TxBuf[2][3]=171;
//				SM2200TxBuf[2][4]=len;
//				for(i=0;i<len;i++)
//				{
//					if(i<85)
//						SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//					if((i>=85)&&(i<170))
//						SM2200TxBuf[1][i-80]=RJ45_1_RData[i+2];
//					if(i>=170)
//						SM2200TxBuf[2][i-165]=RJ45_1_RData[i+2];
//				}
//				for(i=0;i<3;i++)
//				{
//					ChannelSend |=1<<i;
//					ChannelSize [i]=90;
//					ChannelType [i]=0;
//				}	
//			}
//			for(i=0;i<10;i++)
//			{
//				for(k=0;k<18;k++)
//				{
//					SM2200TxBuf[k][0]=0;
//					SM2200TxBuf[k][1]=2;
//					for(j=0;j<88;j++)
//					{
//						SM2200TxBuf[k][j]=15;
//					}
//					ChannelSend |=1<<k;
//					ChannelSize[k]=60;
//					ChannelType[k]=0;
//				}
//				SM2200_Send();
//				delay_ms(300);
//			}
//		}
//	}
//	if(Device==2)
//	{
//		if(SM2200ReceiveFalg!=0)
//		{
//			TIM3->CNT=0;
//			while(TIM3->CNT<500);   //延时20ms
//			SM2200ReceiveFalg=0;
//			len=SM2200RxBuf[0][3];
//			if(len<=85)
//			{
//				for(i=0;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//			if((len>85)&&(len<=170))
//			{
//				for(i=0;i<85;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				for(i=85;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[1][i-80];	
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//			if(len>170)
//			{
//				for(i=0;i<85;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[0][5+i];				
//				}
//				for(i=85;i<170;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[1][i-80];	
//				}
//				for(i=170;i<len;i++)
//				{
//					RJ45_1_WData[i]=SM2200RxBuf[2][i-165];	
//				}
//				RJ45_1_Write(RJ45_1_WData,len);
//			}
//		}
//		if(RJ45_1_ReceiveFlag==1)
//		{
//			RJ45_1_ReceiveFlag=0;
//			len=RJ45_1_RData[0]*256+RJ45_1_RData[1];
//			if(len<=85)
//			{
//				SM2200TxBuf[0][0]=0;
//			  SM2200TxBuf[0][1]=4;      //地址位
//			  SM2200TxBuf[0][2]=len;    //数据包长度
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=len;  
//				for(i=0;i<len;i++)
//				{
//					SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//				}
//				ChannelSend |=1<<0;
//			  ChannelSize [0]=90;
//			  ChannelType [0]=0;
//			}
//			if((len>85)&&(len<=170))
//			{
//				for(i=0;i<2;i++)
//				{
//					SM2200TxBuf[i][0]=0;
//					SM2200TxBuf[i][1]=4;      //地址位
//					SM2200TxBuf[i][2]=len;    //数据包长度
//				}
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=85;  
//				SM2200TxBuf[1][3]=86;
//				SM2200TxBuf[1][4]=len;
//        for(i=0;i<len;i++)
//				{
//					if(i<85)
//						SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//					else
//						SM2200TxBuf[1][i-80]=RJ45_1_RData[i+2];
//				}
//				for(i=0;i<2;i++)
//				{
//					ChannelSend |=1<<i;
//					ChannelSize [i]=90;
//					ChannelType [i]=0;
//				}	
//			}
//			if(len>170)
//			{
//				for(i=0;i<3;i++)
//				{
//					SM2200TxBuf[i][0]=0;
//					SM2200TxBuf[i][1]=4;      //地址位
//					SM2200TxBuf[i][2]=len;    //数据包长度
//				}
//				SM2200TxBuf[0][3]=1;
//				SM2200TxBuf[0][4]=85;  
//				SM2200TxBuf[1][3]=86;
//				SM2200TxBuf[1][4]=170;
//				SM2200TxBuf[2][3]=171;
//				SM2200TxBuf[2][4]=len;
//				for(i=0;i<len;i++)
//				{
//					if(i<85)
//						SM2200TxBuf[0][i+5]=RJ45_1_RData[i+2];
//					if((i>=85)&&(i<170))
//						SM2200TxBuf[1][i-80]=RJ45_1_RData[i+2];
//					if(i>=170)
//						SM2200TxBuf[2][i-165]=RJ45_1_RData[i+2];
//				}
//				for(i=0;i<3;i++)
//				{
//					ChannelSend |=1<<i;
//					ChannelSize [i]=90;
//					ChannelType [i]=0;
//				}	
//			}
//			SM2200_Send ();
//			ChannelSend=0;
//		}
//	}
//}
