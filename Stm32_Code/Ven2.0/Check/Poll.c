#include "Poll.h"
/**
*@brief	��ѯ���ͺ��������豸��ѯ������Ϣ�����豸�����豸�ȴ����豸����Ϣ
*@param Num: ���豸�ӷ�����
*/
extern volatile u8 DeviceScale;             //���Ӵ�������   0�� 1��
extern volatile u8 ConnectDevice[7];        //�����豸��
extern volatile u8 SM2200TxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
extern volatile u8 ChannelSize[18];         //��ͨ�����ݳ���
extern volatile u8 ChannelType[18];         //��ͨ������ģʽ
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
extern volatile u8 SM2200ReceiveFalg;       //��������ʱ���ձ��
extern volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
extern volatile u8 Device;                  //�����豸��   
extern u8 Voltage;                          //��¼��ѹ��ֵ
extern u8 RunMode;                          //�豸����ģʽ
extern u32 ReceNum[7][19];                  //��¼������豸�ķ��͡����մ���
extern volatile u8 ReDevice;                //���յ����ݵ��豸��
void Poll(u8 Num)
{
	u8 i,j,l,k=0;
	u16 time;
	if(DeviceScale==1)     //���豸
	{
		for(i=0;i<7;i++)
		{
			OneAgain:		  if(RunMode==1)               //�ж��Ƿ�����ѯģʽ
			{
				if(ConnectDevice[i]!=0)    //������豸���������ز�ͨ��
				{
					ChannelSend=0x03ffff;
					/*******�������ݰ��齨***********/
					for(j=0;j<18;j++)
					{
						SM2200TxBuf[j][0]=ConnectDevice[i];   //��ַ
						SM2200TxBuf[j][1]=Device;             //������ַ
						SM2200TxBuf[j][2]=Voltage;            //�����ź�ǿ�ȷ�ֵ
						for(l=0;l<18;l++)
						{
							SM2200TxBuf[j][3+2*l]=ChannelFrenquence[l];  //����Ƶ��
							if(ChannelSend&(1<<l))
								SM2200TxBuf[j][4+2*l]=1;                  //ͨ������ʹ��λ
							else
								SM2200TxBuf[j][4+2*l]=0;                  //ͨ������ʹ��λ
						}
						SM2200TxBuf[j][39]=((ReceNum[i][0]+1)&0xFF000000)>>24;
						SM2200TxBuf[j][40]=((ReceNum[i][0]+1)&0x00FF0000)>>16;
						SM2200TxBuf[j][41]=((ReceNum[i][0]+1)&0x0000FF00)>>8;
						SM2200TxBuf[j][42]=((ReceNum[i][0]+1)&0x000000FF);
						ChannelSize[j]=43;
						ChannelType[j]=0;
					}
          SM2200_Send ();
		      ChannelSend=0;
          TIM3->CNT=0;                 //���TIM3������			
				  while(SM2200ReceiveFalg==0)	 //�ȴ��Է�����Ӧ�����ݣ����0.5s���޷��أ�����ѭ��
				  {
						time= TIM3->CNT;
						if(time>8000)
							break;
				  }				
					k++;
          if((SM2200ReceiveFalg==0)&&(k<Num))	
					{
						ReceNum[i][0]+=1;       //���ʹ�����һ
						goto OneAgain;
					}
					ReceNum[i][0]+=1;       //���ʹ�����һ					
          if(SM2200ReceiveFalg!=0)
					{
						TIM3->CNT=0;
						while(ChannelReceive!=0x03ffff)   //�ȴ����ݽ�����ɣ�20ms�����Բ��ܽ�����ɣ�����
						{
							time=TIM3->CNT;
							if(time>200)
								break;
						}
						for(j=0;j<18;j++)                 //���ݽ��д���ͳ��
						{
							if(ChannelReceive&1<<j)
							{
								for(l=0;l<18;l++)
								{
									if(SM2200RxBuf[j][4+2*l]==1)
										ReceNum[i][l+1]+=1;     //ͳ�ƽ��յ�����
								}
								break;
							}
						}
						SM2200ReceiveFalg=0;             //��ռ���
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
	if(DeviceScale==0)    //���豸
	{
		while(SM2200ReceiveFalg==0)   //�ȴ����豸�Դ˷�������
		{
			if(RunMode!=1)              //ģʽ�ı䣬����ѭ��
				break;               
		}
		if(SM2200ReceiveFalg!=0)      //�����ݽ��յ�
		{
			TIM3->CNT=0;
			while(ChannelReceive!=0x03ffff)   //�ȴ����ݽ�����ɣ�30ms�����Բ��ܽ�����ɣ�����
			{
				time=TIM3->CNT;
				if(time>300)
					break;
			}
			/*******���ݽ��������ݰ��齨***********/
			for(i=0;i<18;i++)
			{
				if(ChannelReceive&(1<<i))
				{
					ReDevice=SM2200RxBuf[i][1];
					if(ReDevice<Device)
					{
						ReceNum[ReDevice-1][0]=(SM2200RxBuf[i][39]<<24)+(SM2200RxBuf[i][40]<<16)+(SM2200RxBuf[i][41]<<8)+SM2200RxBuf[i][42];
					}
					else
					{
						ReceNum[ReDevice-2][0]=(SM2200RxBuf[i][39]<<24)+(SM2200RxBuf[i][40]<<16)+(SM2200RxBuf[i][41]<<8)+SM2200RxBuf[i][42];
					}
					break;
				}
			}
			for(i=0;i<18;i++)
			{
				if(ChannelReceive&(1<<i))
				{
					ReceNum[ReDevice-1][i+1]+=1;     //ͳ�ƽ��յ�����
				}
				//���ݰ��齨
				SM2200TxBuf[i][0]=ReDevice;   //��ַ
				SM2200TxBuf[i][1]=Device;             //������ַ
				SM2200TxBuf[i][2]=Voltage;            //�����ź�ǿ�ȷ�ֵ
				for(l=0;l<18;l++)
				{
					SM2200TxBuf[i][3+2*l]=ChannelFrenquence[l];  //����Ƶ��
					if(ChannelReceive&(1<<l))
						SM2200TxBuf[i][4+2*l]=1;                  //ͨ������ʹ��λ
					else
						SM2200TxBuf[i][4+2*l]=0;                  //ͨ������ʹ��λ
				}
				SM2200TxBuf[i][39]=(ReceNum[ReDevice-1][0]&0xFF000000)>>24;
				SM2200TxBuf[i][40]=(ReceNum[ReDevice-1][0]&0x00FF0000)>>16;
				SM2200TxBuf[i][41]=(ReceNum[ReDevice-1][0]&0x0000FF00)>>8;
				SM2200TxBuf[i][42]=(ReceNum[ReDevice-1][0]&0x000000FF);
				ChannelSize[i]=43;
				ChannelType[i]=0;
			}
			ChannelSend=0x03ffff;
			ChannelReceive=0;
			SM2200ReceiveFalg=0;
			SM2200_Send ();
		  ChannelSend=0;
		}
	}	
}