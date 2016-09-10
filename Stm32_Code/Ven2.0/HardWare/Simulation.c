#include "Simulation.h"
#include "sm2200.h"
#include "time.h"
#include "led.h"
#include "W5200.h"

extern volatile u8 Device;            //�豸��
extern volatile u8 ConnectDevice;     //�����豸��
extern volatile u8 DeviceScale;       //���Ӵ�������   0�� 1��
extern u8 RJ45_2_Connect;             //����״̬
extern u8 RunMode;                    //�豸����ģʽ   ��ѯģʽ��1

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
extern u8 RJ45_2_WData[1024];
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
u8 ChannelRece[18];  //  �ӷ���¼����ͨ����

extern u8 FConnectDevice;  //�����豸��
extern u8 FScale;          //�����豸�Ŵ�������
extern u8 FStartVoltage;   //��ʼ�źŷ�ֵ
extern u8 FFlag[33];       //���Ƶ���Ƿ񱻲��Թ�
extern u8 FReceNum[33];    //��Ǹ�Ƶ����մ���
extern u8 FNosie[33];      //��Ǹ�Ƶ������
extern u8 FFrequence[18];  //ʹ��Ƶ��ֵ
u8 FindMode;  // 0��һ�ν�������  1 �Է��豸�����
extern volatile u32 SendTem;
/**
*@brief		ͨ��Ƶ�����Ƶ�㲿�ֳ�ʼ��
*@brief   ��ʼƵ��1,3,5,7,9,11,13,15,16,17,19,21,23,25,27,29,31,33
*/
void FrequenceInit(void)
{
	u8 i,j;
	for(i=0;i<18;i++)     //ͨ��Ƶ����ҳ�ʼ�ź�Ƶ������
	{
		if(i<8)
			FFrequence[i]=2*i+1;
		if(i==8)
			FFrequence[i]=16;
		if(i>8)
			FFrequence[i]=2*i-1;	
	}
//	for(i=1;i<34;i++)  //���ͨ���Ƿ񾭹����� 1���Թ� 0û���Թ�
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
	for(i=0;i<18;i++)     //ת����SM2400ͨ��Ƶ��
	{
		ChannelFrenquence[i]=FFrequence[j]*3+1;
	}
	SetSm2200Frenquence(18);   //18��ͨ������
}
/**
  * ���ܣ�����ͨ�Ų���Ƶ�㣬
  * ������StartVolatage ��ʼ��ѹ����
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
u8 FindChatChannel()
{
	u8 i,j,flag;
	if(FScale==1)  //��ģʽ
	{
		FindMode=0;
		flag=MasterCheck_1(FStartVoltage,FConnectDevice);
		if(RJ45_2_Connect==1)     //������м���������״̬
		{
			RJ45_2_WData[0]=0x3C;   //���ݰ�ͷ
			RJ45_2_WData[1]=0x04;   //Ӧ���ַ
			RJ45_2_WData[3]=0;      //��ʼ��������ͨ��
			RJ45_2_WData[7]=0x3E;   //���ݰ�β
			RJ45_2_Write(RJ45_2_WData,8);
		}
		if(flag==1)   //����ʧ��
			return 1;
		SetVoltages();
		MasterCheck_2(FConnectDevice);
	}
	if(FScale==0)  //��ģʽ
	{
		SlaveCheck();
	}
}

/**
  * ���ܣ���һ������Ƶ�����
  * ������StartVolatage ��ʼ��ѹ����
  * ������ToDevice ��������ӵ��豸��
  * ���أ�0 ͨ����ͨ
 **/
//��Ϣ����  0�����豸��ַ 1����ַ 2��ѹ��ֵ 3~38ͨ����Ϣ  39-74 �ŵ�����   75-77����ͨ��
u8 MasterCheck_1(u8 StartVolatage, u8 ToDevice)
{
	u8 i,j;
	Voltage=StartVolatage;
	RJ45_2_WData[5]=0;      //1�ɹ� 0 ʧ��
	if(Voltage<MaxVoltage1)    //18��ͨ��
	{
		while(Voltage<MaxVoltage1)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x03ffff;    //18��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
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
		if(Voltage<MaxVoltage1)   //�����ݽ��յ������ֳɹ�
		{		
			RJ45_2_WData[4]=18;     //18��ͨ��ģʽ
			RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
			SM2200ReceiveFalg=0;
			ChannelReceive=0; 
			ChannelSend=0;
			return 0;
		}
	}
	
	if((Voltage>=MaxVoltage1)&&(Voltage<MaxVoltage2)) //9��ͨ��
	{
		while(Voltage<MaxVoltage2)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x015555;    //9��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
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
			RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
			RJ45_2_WData[4]=9;     //9��ͨ��ģʽ
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else              //��������9��ͨ����������ѹ
		{
			SendTem =0x02AAAA;    //9��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
				RJ45_2_WData[4]=9;     //9��ͨ��ģʽ
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
		}
	}
	
	if((Voltage>=MaxVoltage2)&&(Voltage<MaxVoltage3)) //6��ͨ��
	{
		while(Voltage<MaxVoltage3)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x9249;    //6��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
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
			RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
			RJ45_2_WData[4]=6;     //6��ͨ��ģʽ
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else
		{
			SendTem =0x12492;    //6��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨	
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
				RJ45_2_WData[4]=6;     //9��ͨ��ģʽ
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
			Delayms(100);
		}
		if(Voltage==MaxVoltage3)   //6��ͨ������
		{
			SendTem = SendTem<<1;    //6��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨	
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
			if(SM2200ReceiveFalg!=0)
			{
				RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
				RJ45_2_WData[4]=6;     //9��ͨ��ģʽ
				SM2200ReceiveFalg=0;
				ChannelReceive=0;  
				return 0;
			}
			Delayms(100);
		}
	}
	if((Voltage>=MaxVoltage3)&&(Voltage<MaxVoltage4)) //3��ͨ��
	{
		while(Voltage<MaxVoltage4)
		{
			OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
			SendTem =0x1041;    //6��ͨ������
			ChannelSend=SendTem;
			MSetInformation();    //���ݰ��齨
			SM2200_Send();        //���ݷ���
			SM2200ReceiveFalg=0;
			MWaitAck();  //�ȴ�Ӧ��
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
			RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
			RJ45_2_WData[4]=3;     //18��ͨ��ģʽ
			SM2200ReceiveFalg=0;
			ChannelReceive=0;  
			return 0;
		}
		else
		{
			for(i=0;i<5;i++)
			{
				SendTem = SendTem<<1;    //6��ͨ������
				ChannelSend=SendTem;
				MSetInformation();    //���ݰ��齨	
				SM2200_Send();        //���ݷ���
				SM2200ReceiveFalg=0;
				MWaitAck();  //�ȴ�Ӧ��
				if(SM2200ReceiveFalg!=0)
				{
					RJ45_2_WData[5]=1;      //1�ɹ� 0 ʧ��
					RJ45_2_WData[4]=3;     //9��ͨ��ģʽ
					SM2200ReceiveFalg=0;
					ChannelReceive=0;  
					return 0;
				}
				Delayms(100);
			}
		}
	}	
	return 1;  //Ƶ�㲻ͨ
}
//�趨�����趨��ѹ
void SetVoltages(void)
{
	if(Voltage<=MaxVoltage1)     //18��ͨ��
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
	if((Voltage<=MaxVoltage2)&&(Voltage>MaxVoltage1))    //9��ͨ��
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
	if((Voltage<=MaxVoltage3)&&(Voltage>MaxVoltage2))  //6��ͨ��
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
	if((Voltage<=MaxVoltage4)&&(Voltage>MaxVoltage3))   //3��ͨ��
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
  * ���ܣ��ڶ�������Ƶ����ң�ȷ������Ƶ��
  * ���أ�0 ͨ����ͨ
 **/
u8 MasterCheck_2(u8 ToDevice)
{
	u8 i,j;
	for(j=0;j<18;j++)
	{
		if(SendTem&(1<<j))   //��ͨ������֤
			FFlag[(ChannelFrenquence[j]-1)/3]=1;
	}
	for(i=0;i<3;i++)
	{
		ChannelSend=SendTem;
		MSetInformation();    //���ݰ��齨
		SM2200_Send();        //���ݷ���
		SM2200ReceiveFalg=0;
		MWaitAck();  //�ȴ�Ӧ��
		if(ChannelReceive!=0)     //���ݰ�����
		{
			for(j=0;j<18;j++)
			{
				if(ChannelReceive&(1<<j))   //��ͨ����ֵ
					FReceNum[(ChannelFrenquence[j]-1)/3]+=1;
			}
		}
		ChannelReceive=0;
		Delayms(80);     //��ʱ80ms
	}
}


/**
  * ���ܣ��ڶ������ַ�����Ϣ����
  * ���أ�0 ִ�гɹ�
 **/
u8 DealInformation_1(void)
{
	u8 i,j,k;
	for(i=0;i<18;i++)
	{
		
	}
//	MReceiveChannelNum=0;     
//	SReceiveChannelNum =0;
//	for(i=0;i<18;i++)                  //18��ͨ�����ռ���ͨ��Ϣ
//	{
//		if((ChannelReceive)&(1<<i))      //��ͨ����ֵ
//		{
//			
//			OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
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
//	//��������ֵ����,������
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
//		RelayNosie[Index[i]]=MChannelNoise[i];            //�����м�
//		RelayFrequence[Index[i]]=MChannelFrequence[i];        //Ƶ���м�
//	}
//	for(i=0;i<MReceiveChannelNum;i++)
//	{
//		MChannelNoise[i]=RelayNosie[i];
//		MChannelFrequence[i]=RelayFrequence[i];
//	}
//	//��������ֵ����,���շ�
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
//		RelayNosie[Index[i]]=SChannelNoise[i];            //�����м�
//		RelayFrequence[Index[i]]=SChannelFrequence[i];        //Ƶ���м�
//	}
//	for(i=0;i<SReceiveChannelNum;i++)
//	{
//		SChannelNoise[i]=RelayNosie[i];
//		SChannelFrequence[i]=RelayFrequence[i];
//	}
//	ChannelNum_1=0;
//	//ͨ��Ƶ������
//	if(SReceiveChannelNum<M_SChannelNum)      //���Դӵ�ͨ������
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
//	if(MReceiveChannelNum<S_MChannelNum)      //�Ӷ�����ͨ������
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
  * ���ܣ����豸����ͨ��Ƶ�����ʱ����Ӧ��
 **/
void SlaveCheck(void)
{
	u8 i,j;
	u16 k;
	while(SM2200ReceiveFalg==0)
	{
		if(RunMode!=3) //����ģʽ�ı䣬����ѭ��
			break;
	}
	if(SM2200ReceiveFalg!=0)  //������
	{
		for(i=0;i<18;i++)
		{
			if(ChannelReceive&1<<i)  //�ҵ����ݰ������н���
			{
				ConnectDevice=SM2200RxBuf[i][1];
				Voltage =SM2200RxBuf[i][2];
				OfdmXcvrWrite(TX_OUT_VOLTAGE,2,Voltage);
				FindMode =SM2200RxBuf[i][3];
				if(FindMode==1)   //�����ͨ��Ƶ�����á�
				{
					for(j=0;j<18;j++)
						ChannelFrenquence[j]=SM2200RxBuf[i][4+2*j];
					SetSm2200Frenquence(18);   //18��ͨ������
				}
				ChannelSend=(SM2200RxBuf[i][58]<<16)+(SM2200RxBuf[i][59]<<8)+SM2200RxBuf[i][60];
				break;
			}
		}
		ReadRoise();
		SSetInformation();
		SM2200_Send();        //���ݷ���
	}
}
/**
  *���ܣ����豸�齨�ز��������ݰ�
  *��ʽ��//ͨ��Ƶ�����4-39��40-57�ŵ�������� 58 59 60 ����ͨ�����  
 **/
void MSetInformation(void)
{
	u8 i,j;
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][0]=FConnectDevice;
		SM2200TxBuf[i][1]=Device;              //�豸��
	  SM2200TxBuf[i][2]=Voltage;             //�ź�ǿ��
		SM2200TxBuf[i][3]=FindMode ;           //���з�ʽ
		for(j=0;j<18;j++)                      //ͨ��Ƶ�����4-39��40-57�ŵ�������� 58 59 60 ����ͨ�����                     
		{
			SM2200TxBuf[i][4+j*2]=ChannelFrenquence[j];
			SM2200TxBuf[i][5+j*2]=0;
		}	                              //Ƶ����Ϣ	
		ChannelSize[i]=78;              //���ݳ���          
		ChannelType[i]=0;              //��������
			
		SM2200TxBuf[i][58]=ChannelSend/65536;
		SM2200TxBuf[i][59]=(ChannelSend%65536)/256;
		SM2200TxBuf[i][60]=ChannelSend%256;	  			
	}
}

/**
  *���ܣ����豸�ȴ����豸Ӧ��
 **/
void MWaitAck(void)
{
	TIM3->CNT=0;
	while(TIM3->CNT<5000)    //��ʱ500ms
	{
		if(SM2200ReceiveFalg!=0)
			break;
	}
	TIM3->CNT=0;
	while(TIM3->CNT<500);   //��ʱ50ms
}
/**
  *���ܣ���ʱ����
 **/
void Delayms(u16 ms)
{
	TIM3->CNT=0;
	ms=ms*10;
	while(TIM3->CNT<ms)    //��ʱ500ms
	{	
	}
}
/**
  *���ܣ���ȡ�ŵ�����,���Ҷ�ȡ���յ����ŵ���
 **/
void ReadRoise(void)	
{
	u8 i;
	u16 j;
	for(i=0;i<18;i++)
	{
		OfdmXcvrWrite(CLUSTER_SELECT,2,i);    //ͨ��ѡ��
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
  *���ܣ����豸�齨�ز��������ݰ�
  *��ʽ��//ͨ��Ƶ�����4-39��40-57�ŵ�������� 58 59 60 ����ͨ�����  
 **/
void SSetInformation(void)
{
	u8 i,j;
	//���ݰ��齨
	for(i=0;i<18;i++)
	{
		SM2200TxBuf[i][0]=ConnectDevice;
		SM2200TxBuf[i][1]=Device;              //�豸��
		SM2200TxBuf[i][2]=Voltage;             //�ź�ǿ��
		SM2200TxBuf[i][3]=FindMode ;           //�ź�ǿ��
		for(j=0;j<18;j++)                      //ͨ��Ƶ�����4-39��40-57�ŵ�������� 58 59 60 ����ͨ�����                     
		{
			SM2200TxBuf[i][4+j*2]=ChannelFrenquence[j];
			SM2200TxBuf[i][5+j*2]=ChannelRece[j];
			SM2200TxBuf[i][40+j]=SChannelNoise[j];
		}
		SM2200TxBuf[i][58]=ChannelSend/65536;
		SM2200TxBuf[i][59]=(ChannelSend%65536)/256;
		SM2200TxBuf[i][60]=ChannelSend%256;	 
		ChannelSize[i]=78;              //���ݳ���          
		ChannelType[i]=0;              //��������		
	}
}

