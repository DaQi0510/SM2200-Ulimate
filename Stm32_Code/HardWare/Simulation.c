#include "Simulation.h"
#include "sm2200.h"
#include "time.h"
#include "led.h"
#include "W5200.h"

extern volatile u8 Device;            //�豸��
extern volatile u8 ConnectDevice;     //�����豸��
extern volatile u8 DeviceScale;       //���Ӵ�������   0�� 1��

extern volatile u8 SM2200TxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
extern volatile u8 ChannelSize[18];         //��ͨ�����ݳ���
extern volatile u8 ChannelType[18];         //��ͨ������ģʽ
extern volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
extern volatile u8 SM2200ReceiveFalg;       //��������ʱ���ձ��
extern volatile u16 Noise[18];              //��¼ͨ������
extern volatile u8  ShakeChannel[18];       //��¼ͨ������
extern u8 RJ45_1_WData[1024];
extern u8 Voltage;                      //��¼��ѹ��ֵ

/****************��һ�ַ���ͨ��ͨ����ر���*********************/
u8 volatile MChannelFrequence[18];     //���ͷ����յ�Ƶ������ 
u8 volatile SChannelFrequence[18];     //���շ����յ�Ƶ������ 
u8 volatile MChannelNoise[18];         //���ͷ����յ�Ƶ������
u8 volatile SChannelNoise[18];         //���շ����յ�Ƶ������
u8 volatile SReceiveChannelNum;        //���շ����յ�ͨ����
u8 volatile MReceiveChannelNum;        //���ͷ����յ�ͨ����
u8 volatile Index[18];                 //��¼������
u8 volatile RelayNosie[18];            //�����м�
u8 volatile RelayFrequence[18];        //Ƶ���м�
u8 volatile M_SChannelNum=2;           //���ͷ������շ�����ͨ�ŵ�ͨ����Ŀ
u8 volatile S_MChannelNum=2;           //���շ������ͷ�����ͨ�ŵ�ͨ����Ŀ
u8 volatile ChannelNum_1;              //���ͷ��ͽ��շ�����ռ���ܹ�ͨ����

/**
  * ���ܣ�����ͨ�Ų���Ƶ�㣬
  * ������StartVolatage ��ʼ��ѹ����
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 FindChatChannel()
{
	u8 i,j;
	if(Device==1)    //�������ͷ�
	{
		if(MasterCheck_1(MinVoltage1,4)==1)    //��һ��δ���ҵ�ͨ��ͨ��
		{
			RJ45_1_WData[0]='F';
			RJ45_1_WData[1]='a';
			RJ45_1_WData[3]='i';
			RJ45_1_WData[4]='l';
			RJ45_1_WData[5]=1;
			RJ45_1_Write(RJ45_1_WData,128);
			return 1;
		}	
     		
		else                                            //��һ�η���ͨ��ͨ��
		{
			RJ45_1_WData[0]='O';
			RJ45_1_WData[1]='k';
			RJ45_1_WData[3]=0;
			RJ45_1_Write(RJ45_1_WData,128);
		}
		delay_ms(100);
		if(MasterCheck_2(4)==1)            //�ڶ���δ���ҵ�ͨ��ͨ��     
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
//��һȦ������Ϣ
		while(SM2200ReceiveFalg==0);
		TIM3->CNT=0;
		while(TIM3->CNT<300);
    //������Ϣ��0~1��ַ 2Ƶ�� 3���� 4~6����ͨ��ֵ  
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				Voltage=SM2200RxBuf[j][38];	
			  OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //ͨ��ѡ��
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
    //18ͨ��������Ϣ		
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
//��һȦ������Ϣ
		
//�ڶ�Ȧ������Ϣ
		LED1=1;
//	}
		while(SM2200ReceiveFalg==0);
		TIM3->CNT=0;
		while(TIM3->CNT<300);
    //������Ϣ��0~1��ַ 2Ƶ�� 3���� 4~6����ͨ��ֵ  
		for(j=0;j<18;j++)
		{
			if(ChannelReceive&(1<<j))
			{
				Voltage=SM2200RxBuf[j][38];	
			  OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //ͨ��ѡ��
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
    //18ͨ��������Ϣ		
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
//��һȦ������Ϣ
		
//�ڶ�Ȧ������Ϣ
		LED1=1;
	}
}

/**
  * ���ܣ���һ������Ƶ�����
  * ������StartVolatage ��ʼ��ѹ����
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
//��Ϣ����  0~1�����豸��ַ   2~37ͨ����Ϣ  38 ��ѹ��ֵ  39~41����ͨ��
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice)
{
	u8 i,j;
	Voltage=StartVolatage;
	if(Voltage<MaxVoltage1)    //18��ͨ��
	{
		while(Voltage<MaxVoltage1)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=0;i<18;i++)
			{				
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //Ƶ����Ϣ
				SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
				ChannelSize[i]=45;              //���ݳ���          
				ChannelType [i]=0;              //��������
				ChannelSend|=1<<i;              //����ͨ����
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
			while(TIM3->CNT<6000)    //��ʱ600ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //��ʱ50ms
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
	
	if((Voltage>=MaxVoltage1)&&(Voltage<MaxVoltage2)) //9��ͨ��
	{
		while(Voltage<MaxVoltage2)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=1;i<18;i=i+2)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //Ƶ����Ϣ
				SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
				ChannelSize[i]=45;              //���ݳ���          
				ChannelType [i]=0;              //��������
				ChannelSend|=1<<i;              //����ͨ����
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //��ʱ500ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //��ʱ20ms
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
	
	if((Voltage>=MaxVoltage2)&&(Voltage<MaxVoltage3)) //6��ͨ��
	{
		while(Voltage<MaxVoltage3)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=2;i<18;i=i+3)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //Ƶ����Ϣ
				SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
				ChannelSize[i]=45;              //���ݳ���          
				ChannelType[i]=0;              //��������
				ChannelSend|=1<<i;              //����ͨ����
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //��ʱ500ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //��ʱ20ms
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
	if((Voltage>=MaxVoltage3)&&(Voltage<MaxVoltage4)) //3��ͨ��
	{
		while(Voltage<MaxVoltage4)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			ChannelSend=0;
			for(i=4;i<18;i=i+6)
			{
				SM2200TxBuf[i][0]=ToDevice/256;
				SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
				for(j=0;j<18;j++)
				{
					SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
					SM2200TxBuf[i][3+j*2]=0;
				}	                              //Ƶ����Ϣ
				SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
				ChannelSize[i]=45;              //���ݳ���          
				ChannelType[i]=0;              //��������
				ChannelSend|=1<<i;              //����ͨ����
			}
			for(i=0;i<18;i++)		
			{
				SM2200TxBuf[i][39]=ChannelSend/65536;
				SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
				SM2200TxBuf[i][41]=ChannelSend%256;	  		
			}	
			SM2200_Send();
			TIM3->CNT=0;
			while(TIM3->CNT<6000)    //��ʱ600ms
			{
				if(SM2200ReceiveFalg!=0)
					break;
			}
			TIM3->CNT=0;
			while(TIM3->CNT<500);   //��ʱ20ms
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
	return 1;  //Ƶ�㲻ͨ
}

/**
  * ���ܣ��ڶ�������Ƶ����ң��ڵ�һ�ֻ����ϼ�3
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 MasterCheck_2(u8 ToDevice)
{
	u8 i,j;
	if(Voltage<=MaxVoltage1)     //18��ͨ��
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
			SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //Ƶ����Ϣ
			SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
			ChannelSize[i]=45;              //���ݳ���          
			ChannelType[i]=0;               //��������
			ChannelSend|=1<<i;              //����ͨ����
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //��ʱ500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //��ʱ20ms
	}
	
	if((Voltage<=MaxVoltage2)&&(Voltage>MaxVoltage1))    //9��ͨ��
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
			SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //Ƶ����Ϣ
			SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
			ChannelSize[i]=45;              //���ݳ���          
			ChannelType[i]=0;               //��������
			ChannelSend|=1<<i;              //����ͨ����
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<5000)    //��ʱ500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<200);   //��ʱ20ms
	}
	
	if((Voltage<=MaxVoltage3)&&(Voltage>MaxVoltage2))  //6��ͨ��
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
			SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //Ƶ����Ϣ
			SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
			ChannelSize[i]=45;              //���ݳ���          
			ChannelType[i]=0;               //��������
			ChannelSend|=1<<i;              //����ͨ����
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //��ʱ500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //��ʱ20ms
	}
	
	if((Voltage<=MaxVoltage4)&&(Voltage>MaxVoltage3))   //3��ͨ��
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
			SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
			for(j=0;j<18;j++)
			{
				SM2200TxBuf[i][2+j*2]=ChannelFrenquence[j];
				SM2200TxBuf[i][3+j*2]=0;
			}	                              //Ƶ����Ϣ
			SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
			ChannelSize[i]=45;              //���ݳ���          
			ChannelType[i]=0;               //��������
			ChannelSend|=1<<i;              //����ͨ����
		}
		for(i=0;i<18;i++)		
		{
			SM2200TxBuf[i][39]=ChannelSend/65536;
			SM2200TxBuf[i][40]=(ChannelSend%65536)/256;
			SM2200TxBuf[i][41]=ChannelSend%256;
		}				
		SM2200_Send();
		TIM3->CNT=0;
		while(TIM3->CNT<6000)    //��ʱ500ms
		{
			if(SM2200ReceiveFalg!=0)
				break;
		}
		TIM3->CNT=0;
		while(TIM3->CNT<500);   //��ʱ20ms
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
  * ���ܣ��ڶ������ַ�����Ϣ����
  * ���أ�0 ִ�гɹ�
 **/
u8 DealInformation_1(void)
{
	u8 i,j,k;
	MReceiveChannelNum=0;     
	SReceiveChannelNum =0;
	for(i=0;i<18;i++)                  //18��ͨ�����ռ���ͨ��Ϣ
	{
		if((ChannelReceive)&(1<<i))      //��ͨ����ֵ
		{
			
			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
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
	//��������ֵ����,������
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
		RelayNosie[Index[i]]=MChannelNoise[i];            //�����м�
		RelayFrequence[Index[i]]=MChannelFrequence[i];        //Ƶ���м�
	}
	for(i=0;i<MReceiveChannelNum;i++)
	{
		MChannelNoise[i]=RelayNosie[i];
		MChannelFrequence[i]=RelayFrequence[i];
	}
	//��������ֵ����,���շ�
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
		RelayNosie[Index[i]]=SChannelNoise[i];            //�����м�
		RelayFrequence[Index[i]]=SChannelFrequence[i];        //Ƶ���м�
	}
	for(i=0;i<SReceiveChannelNum;i++)
	{
		SChannelNoise[i]=RelayNosie[i];
		SChannelFrequence[i]=RelayFrequence[i];
	}
	ChannelNum_1=0;
	//ͨ��Ƶ������
	if(SReceiveChannelNum<M_SChannelNum)      //���Դӵ�ͨ������
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
	if(MReceiveChannelNum<S_MChannelNum)      //�Ӷ�����ͨ������
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
  * ���ܣ�����������ȷ����Ϣ
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 MasterCheck_3(u8 ToDevice)
{
	u8 i,j;
	SetSm2200Frenquence(ChannelNum_1);
	ChannelSend=0;
	for(i=0;i<ChannelNum_1;i++)
	{
		SM2200TxBuf[i][0]=ToDevice/256;
		SM2200TxBuf[i][1]=ToDevice%256;             //�豸��
		for(j=0;j<ChannelNum_1;j++)               //Ƶ����Ϣ
		{
			SM2200TxBuf[i][j*2+2]=ChannelFrenquence[j];
		}
		SM2200TxBuf[i][38]=Voltage;     //��ѹ��ֵ		
		ChannelSize[i]=45;              //���ݳ���          
		ChannelType[i]=0;               //��������
		ChannelSend|=1<<i;              //����ͨ����
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
  * ���ܣ���һ������Ƶ�����(��)
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 SlaveCheck_1(u8 Todevice)
{
	u8 i,j;
	while(SM2200ReceiveFalg==0);
	TIM3->CNT=0;
	while(TIM3->CNT<300);
	Voltage=0;
	ChannelSend=0;
	//������Ϣ��0~1��ַ 2Ƶ�� 3���� 4~6����ͨ��ֵ  
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			if(Voltage==0)
			{
				Voltage=SM2200RxBuf[j][38];	
			}
			OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //ͨ��ѡ��
			SChannelNoise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
			SChannelFrequence[j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
      if(ChannelSend==0)
			{
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];		
			}				
		}				
	}
	//18ͨ��������Ϣ		
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
  * ���ܣ��ڶ�������Ƶ�����(��)
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 SlaveCheck_2(u8 Todevice)
{
	u8 i,j;
	while(SM2200ReceiveFalg==0);
	TIM3->CNT=0;
	while(TIM3->CNT<300);
	Voltage=0;
	ChannelSend=0;
	//������Ϣ��0~1��ַ 2Ƶ�� 3���� 4~6����ͨ��ֵ  
	for(j=0;j<18;j++)
	{
		if(ChannelReceive&(1<<j))
		{
			if(Voltage==0)
			{
				Voltage=SM2200RxBuf[j][38];	
			}
			OfdmXcvrWrite(CLUSTER_SELECT,2,j);    //ͨ��ѡ��
			SChannelNoise[j]=OfdmXcvrRead(CARRIER_NOISE,2);
			SChannelFrequence[j]=OfdmXcvrRead(CARRIER_FREQ_SELECT,2);
      if(ChannelSend==0)
			{
				ChannelSend=SM2200RxBuf[j][39]*65536+SM2200RxBuf[j][40]*256+SM2200RxBuf[j][41];		
			}				
		}				
	}
	//18ͨ��������Ϣ		
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

