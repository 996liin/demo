#include "headfile.h"


#define IIC_SCL_H	gpio_bit_set(GPIOB,GPIO_PIN_8)
#define IIC_SCL_L	gpio_bit_reset(GPIOB,GPIO_PIN_8)
#define IIC_SDA_H	gpio_bit_set(GPIOB,GPIO_PIN_9)
#define IIC_SDA_L	gpio_bit_reset(GPIOB,GPIO_PIN_9)

#define READ_SDA	gpio_input_bit_get(GPIOB,GPIO_PIN_9)

static void AT24CXX_Init(void);
static void AT24CXX_Read(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead);
static void AT24CXX_Write(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite);

static Struct_AT24CXX s_AT24CXX = 
{
	.AT24CXX_Init = AT24CXX_Init,
	.AT24CXX_Read = AT24CXX_Read,
	.AT24CXX_Write = AT24CXX_Write,
	
};
Struct_AT24CXX *ps_AT24CXX = &s_AT24CXX;

//初始化IIC
static void IIC_Init(void)
{	
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_8);
}
static void SDA_IN(void)
{
  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
}
static void SDA_OUT(void)
{
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
}
static void IIC_Start(void)
{
	SDA_OUT();
	IIC_SCL_L;
	
	ps_Systick->delay_us(1);
	IIC_SDA_H;
	ps_Systick->delay_us(1);
	IIC_SCL_H;
	ps_Systick->delay_us(4);
	IIC_SDA_L;
	ps_Systick->delay_us(4);
	IIC_SCL_L;
}
static void IIC_Stop(void)
{
	SDA_OUT();				//sda线输出
	IIC_SCL_L;
	ps_Systick->delay_us(1);
	IIC_SDA_L;				//STOP:when CLK is high DATA change form low to high
 	ps_Systick->delay_us(4);
	IIC_SCL_H;
	ps_Systick->delay_us(4);
	IIC_SDA_H;				//发送I2C总线结束信号
	ps_Systick->delay_us(4);							   	
}

static unsigned char IIC_Wait_Ack(void)
{
	unsigned char ucErrTime=0;

	SDA_IN();      //SDA设置为输入  
	IIC_SDA_H;
	ps_Systick->delay_us(1);	   
	IIC_SCL_H;
	ps_Systick->delay_us(1);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_L;		//时钟输出0 	   
	return 0;  
} 
static void IIC_Ack(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_L;
	ps_Systick->delay_us(4);
	IIC_SCL_H;
	ps_Systick->delay_us(4);
	IIC_SCL_L;
}
static void IIC_NAck(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_H;
	ps_Systick->delay_us(4);
	IIC_SCL_H;
	ps_Systick->delay_us(4);
	IIC_SCL_L;
}					 				     			  
static void IIC_Send_Byte(unsigned char txd)
{                        
    unsigned char t;   
	
	SDA_OUT(); 	    
    IIC_SCL_L;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
		if((txd&0x80)>>7==1)
			IIC_SDA_H;
		else
			IIC_SDA_L;
			
        txd<<=1; 	  
		ps_Systick->delay_us(4);   //对TEA5767这三个延时都是必须的
		IIC_SCL_H;
		ps_Systick->delay_us(4); 
		IIC_SCL_L;	
		ps_Systick->delay_us(4);
    }	 
} 	    
static unsigned char IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();					//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL_L; 
        ps_Systick->delay_us(4);
		IIC_SCL_H;
        receive<<=1;
        if(READ_SDA)receive++;   
		ps_Systick->delay_us(2); 
    }					 
    if (!ack)
        IIC_NAck();				//发送nACK
    else
        IIC_Ack(); 				//发送ACK   
    return receive;
}
static unsigned char AT24CXX_ReadOneByte(unsigned short ReadAddr)
{				  
	unsigned char temp=0;		  	    																 
    IIC_Start();  
	
	IIC_Send_Byte(0XA0);			//发送写命令
	IIC_Wait_Ack();
	IIC_Send_Byte(ReadAddr/256);	//发送高地址
	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);	//发送低地址
	IIC_Wait_Ack();	  
	
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);			//进入接收模式			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();						//产生一个停止条件	    
	return temp;
}

static void AT24CXX_WriteOneByte(unsigned short WriteAddr,unsigned char DataToWrite)
{				   	  	    																 
    IIC_Start();  
	
	IIC_Send_Byte(0XA0); 	    	//发送写命令
	IIC_Wait_Ack(); 
	IIC_Send_Byte(WriteAddr/256);	//发送高地址 
	IIC_Wait_Ack();	   
	IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 
	
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();						//产生一个停止条件 
	
	ps_Systick->delay_ms(20);	 
} 

static unsigned char AT24CXX_Check(void)
{
	unsigned char temp;
	temp=AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX			   
	
	if(temp==0X55)
	{
		return 0;		   
	}
	else						//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0X55)
		{
			return 0;
		}
	}
	return 1;											  
}
static void AT24CXX_Init(void)
{
	IIC_Init();
	AT24CXX_Check();
}
static void AT24CXX_Read(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  

static void AT24CXX_Write(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}
 

