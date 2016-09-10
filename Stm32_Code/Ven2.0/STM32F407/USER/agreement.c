#include "agreement.h"

extern unsigned int TIM3_ms;

/******************************************************************************/
/*                           'INT'命令处理                                    */
/*                             错误返回0                                      */
/*                             成功返回1                                      */
/******************************************************************************/

unsigned char deal_int(void)
{
	unsigned long len;
	unsigned char command[8];
	
	TIM3_ms=0;//定时清零
	while(1)
	{
		if(TIM3_ms>=2000)//2秒超时
		{
			return 0;//失败
		}
		len=recv_len();//收到数据长度
		if(len>0)
		{
			recv_data(command,len);//接收数据
			if(command[0]=='I'&&command[1]=='N'&&command[2]=='T')//收到'INT'
			{
				command[0]='A';//'ACK'应答
				command[1]='C';
				command[2]='K';
				command[4]=1;//转速测点数
				command[5]=6;//温度测点数
				command[6]=1;//温湿度测点数
				command[7]=4;//振动通道数
				send_data(command,8);//发送应答
				
				return 1;//成功
			}
		}
	}
	
}
	
/******************************************************************************/
/*                           'PRE'命令处理                                    */
/*                             错误返回0                                      */
/*                             成功返回1                                      */
/******************************************************************************/

unsigned char pre;
unsigned char deal_pre(void)
{
	unsigned long len;
	unsigned char command[8];
	
	TIM3_ms=0;//定时清零
	while(1)
	{
		if(TIM3_ms>=2000)//2秒超时
		{
			return 0;//失败
		}
		len=recv_len();//收到数据长度
		if(len>0)
		{
			recv_data(command,len);//接收数据
			if(command[0]=='P'&&command[1]=='R'&&command[2]=='E')//收到'PRE'
			{
				pre=command[7];//获取预分频值
				
				command[0]='A';//'ACK'应答
				command[1]='C';
				command[2]='K';
				send_data(command,8);//发送应答
				
				return 1;//成功
			}
		}
	}
}
	
/******************************************************************************/
/*                           'DIV'命令处理                                    */
/*                             错误返回0                                      */
/*                             成功返回1                                      */
/******************************************************************************/

unsigned char div[4];
unsigned char deal_div(void)
{
	unsigned char i = 0;
	unsigned long len;
	unsigned char command[8];
	
	i=0;
	TIM3_ms=0;//定时清零
	while(1)
	{
		if(TIM3_ms>=2000)//2秒超时
		{
			return 0;//失败
		}
		len=recv_len();//收到数据长度
		if(len>0)
		{
			recv_data(command,len);//接收数据
			if(command[0]=='D'&&command[1]=='I'&&command[2]=='V')//收到'DIV'
			{
				div[command[6]]=command[7];//通道分频值
				
				command[0]='A';//'ACK'应答
				command[1]='C';
				command[2]='K';
				send_data(command,8);//发送应答
				
				i++;
				if(i>=4)//4通道
				{
					return 1;//成功
				}
			}
		}
	}
}
	
/******************************************************************************/
/*                           'STA'命令处理                                    */
/*                             错误返回0                                      */
/*                             成功返回1                                      */
/******************************************************************************/

unsigned char deal_sta(void)
{
	unsigned long len;
	unsigned char command[8];
	
	while(1)
	{
		if(TIM3_ms>=2000)//2秒超时
		{
			return 0;//失败
		}
		len=recv_len();//收到数据长度
		if(len>0)
		{
			recv_data(command,len);//接收数据
			if(command[0]=='S'&&command[1]=='T'&&command[2]=='A')//收到'STA'
			{
				return 1;//成功
			}
		}
	}
}
	
/******************************************************************************/
/*                             数据传输                                       */
/*                             错误返回0                                      */
/*                             成功返回1                                      */
/******************************************************************************/	

unsigned char package[5000];
extern unsigned char  ad1_buffer[Max_Buffer],
											ad2_buffer[Max_Buffer],
											ad3_buffer[Max_Buffer],
											ad4_buffer[Max_Buffer];

unsigned char * p_package = package;
unsigned char * p_ad1_buffer = ad1_buffer,
							* p_ad2_buffer = ad2_buffer,
							* p_ad3_buffer = ad3_buffer,
							* p_ad4_buffer = ad4_buffer;

unsigned short package_num;
unsigned int dw;

unsigned char deal_data(void)
{
	unsigned int i;
	unsigned char state;
	
	w5300_read(&state,Sn_SSR1(0),1);//检测连接是否断开
	if(state!=SOCK_ESTABLISHED)
	{
		return 0;//失败
	}
	
	p_package = package;//数据包指针
	
	*p_package++ = 'D';//包头
	*p_package++ = 'A';
	*p_package++ = 'T';
	
	*p_package++ = (package_num)>>8;//包序号高八位
	*p_package++ = package_num;//包序号低八位
	package_num++;//包序号+1
	if(package_num>93750/(pre+1)/500)//包序号1~93750/(pre+1)/500
	{
		package_num=1;
	}
	
	while(dw<=1000);//等待内存具有足够存储深度
	
	for(i=0;i<1000;i++)//通道1数据写入数据包
	{
		*p_package++ = *p_ad1_buffer++;
	}
	if(p_ad1_buffer - ad1_buffer >= Max_Buffer)//判断通道1指针是否超出缓存
	{
		p_ad1_buffer = ad1_buffer;
	}
	
	for(i=0;i<1000;i++)//通道2数据写入数据包
	{
		*p_package++ = *p_ad2_buffer++;
	}
	if(p_ad2_buffer - ad2_buffer >= Max_Buffer)//判断通道2指针是否超出缓存
	{
		p_ad2_buffer = ad2_buffer;
	}
	
	for(i=0;i<1000;i++)//通道3数据写入数据包
	{
		*p_package++ = *p_ad3_buffer++;
	}
	if(p_ad3_buffer - ad3_buffer >= Max_Buffer)//判断通道3指针是否超出缓存
	{
		p_ad3_buffer = ad3_buffer;
	}
	
	for(i=0;i<1000;i++)//通道4数据写入数据包
	{
		*p_package++ = *p_ad4_buffer++;
	}
	if(p_ad4_buffer - ad4_buffer >= Max_Buffer)//判断通道4指针是否超出缓存
	{
		p_ad4_buffer = ad4_buffer;
	}
	
	dw-=1000;//存储深度-1000
	
	*p_package++ = 0;//转速高八位
	*p_package++ = 0;//转速低八位
	
	for(i=0;i<6;i++)//温度
	{
		*p_package++ = 0;//温度数据整数部分
		*p_package++ = 0;//温度数据小数部分
	}
	
	*p_package++ = 0;//温度部分
	*p_package++ = 0;//湿度部分

	*p_package++ = '_';//包尾
	*p_package++ = 'P';
	*p_package++ = 'S';
	*p_package++ = 'A';
	*p_package++ = 'I';
	if( ( (p_package-package) & 0x01 ) == 1 )//包长度是奇数
	{
		*p_package++ = '_';
	}

	send_data(package, p_package-package);//发送数据包
	
	return 1;//成功
}
