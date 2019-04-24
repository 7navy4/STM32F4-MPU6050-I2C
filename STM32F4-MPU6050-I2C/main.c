#include "stm32f4xx.h"                  // Device header
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
#define ADDRESS 0x68 // MPU6050 slave adress



int16_t xl,xh,yl,yh;
int16_t x,y,xm,ym;

void delay(uint32_t i){
   		while(i)
       {
				i--;										
			 }
}

long map(long x,long in_min,long in_max,long out_min,long out_max)
{
return (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}

void init_I2C1(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;
	
	// enable APB1 clock bus for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	// enable AHB1 clock bus for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/*
	 SCL on PB6 and SDA on PB7 
	 SCL on PB8 and SDA on PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			//set to open drain 
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA
	
	// I2C1 
	I2C_InitStruct.I2C_ClockSpeed = 100000; 		
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;		// enable acknowledge 
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 7 bit address length 
	I2C_Init(I2C1, &I2C_InitStruct);			
	
	// enable I2C1
	I2C_Cmd(I2C1, ENABLE);
}




void write_i2c(I2C_TypeDef* I2Cx,uint8_t address, uint8_t data){


    // wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
 
 	// I2C1 enable
	I2C_GenerateSTART(I2Cx, ENABLE);
	  
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
		
	// Send slave Address for write 
	I2C_Send7bitAddress(I2Cx, address, I2C_Direction_Transmitter);
	
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2Cx, ENABLE);

}


int read_i2c(I2C_TypeDef* I2Cx,uint8_t address)
{
	// wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
	// I2C1 enable
	I2C_GenerateSTART(I2Cx, ENABLE);
	  
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)); 
		
	// Send slave Address for read 
	I2C_Send7bitAddress(I2Cx, address, I2C_Direction_Receiver);
	
	
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	
	I2C_GenerateSTOP(I2Cx, ENABLE);
	
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	
	// read data from I2C data register 
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;

}

int main(void){
	
	init_I2C1(); // initialize I2C peripheral
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12 |GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	

	
	while(1){
		
  	write_i2c(I2C1,ADDRESS<<1,0X3B); // PWR_MGMT_1 register 
		write_i2c(I2C1,ADDRESS<<1,0X3B); // wakes up the MPU-6050

	  write_i2c(I2C1,ADDRESS<<1,0X3B);//ACCEL_XOUT_H 
		xh=read_i2c(I2C1,ADDRESS<<1);
		
		write_i2c(I2C1,ADDRESS<<1,0x3C);//ACCEL_XOUT_L  
		xl=read_i2c(I2C1,ADDRESS<<1);
		
		write_i2c(I2C1,ADDRESS<<1,0x3D);//ACCEL_YOUT_H 
		yh=read_i2c(I2C1,ADDRESS<<1);
		
	  write_i2c(I2C1,ADDRESS<<1,0x3E);//ACCEL_YOUT_L
		yl=read_i2c(I2C1,ADDRESS<<1);
	
	 	x  =((xh<<8 | xl));
	 	y  =((yh<<8)| yl);
			
		xm=map(x,-17000,17000,0,100);
		ym=map(y,-17000,17000,0,100);
			
  	delay(10000);

		
		   if(0<xm & xm<50)
	     {
	      GPIO_SetBits(GPIOD,GPIO_Pin_14);
	     	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	     }
			 
	     if(50<=xm & xm<=100)
		   {
			  GPIO_SetBits(GPIOD, GPIO_Pin_12);
	  		GPIO_ResetBits(GPIOD, GPIO_Pin_14);
			 }
			
			 if(0<ym & ym<50)
			 {
	      GPIO_SetBits(GPIOD,  GPIO_Pin_13);
	    	 GPIO_ResetBits(GPIOD,GPIO_Pin_15);
		   }
			   
		 if(50<=ym & ym<100)
		  {
				GPIO_SetBits(GPIOD, GPIO_Pin_15);
				GPIO_ResetBits(GPIOD,GPIO_Pin_13);
			}
		 
		}
}
	
