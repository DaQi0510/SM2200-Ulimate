#include "DeviceToDevice.h"
extern volatile u8 Device;            //�豸��

extern volatile u8 SM2200TxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 SM2200RxBuf[18][128];    //18��ͨ���������ݰ�����
extern volatile u8 ChannelFrenquence[18];   //18��ͨ��Ƶ������
extern volatile u8 ChannelSize[18];         //��ͨ�����ݳ���
extern volatile u8 ChannelType[18];         //��ͨ������ģʽ
extern volatile u32 ChannelSend;            //��Ƿ��͵�ͨ��
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��
extern volatile u8 SM2200ReceiveFalg;       //��������ʱ���ձ��

void DeviceToDevice(u32 CheckNum)
{
	u8 i;
}
