#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define FOSC 8000000 // Clock Speed
#define BAUD 9600
#define UBRR FOSC/16/BAUD-1

#define LSH_REG_LOW  PORTB&=0xFE
#define LSH_REG_HIGH PORTB|=0x01

#define RSH_REG_LOW  PORTD&=0xEF
#define RSH_REG_HIGH PORTD|=0x10

#define LST_REG_LOW  PORTB&=0xFD
#define LST_REG_HIGH PORTB|=0x02

#define RST_REG_LOW  PORTD&=0xF7
#define RST_REG_HIGH PORTD|=0x08
#define MAX_LEN 100

unsigned char data[][8]={
						  {0xFF,0xFF,0xE3,0xDD,0xFF,0xFF,0xFF,0xFF},/*smeil0*/
						  {0xFF,0xFF,0xC7,0xBB,0xFF,0xFF,0xFF,0xFF},

						  {0xFF,0xFF,0xC3,0xFB,0xFF,0xFF,0xFF,0xFF},/*"jiongpo",0*/
						  {0xFF,0xFF,0xC3,0xDF,0xFF,0xFF,0xFF,0xFF},/*"未命名文件",0*/


						  {0xFF,0xFF,0xC3,0xEF,0xF7,0xC3,0xFF,0xFF},/*"asleep",0*/
						  {0xFF,0xFF,0xC3,0xEF,0xF7,0xC3,0xFF,0xFF},/*"asleep",0*/


                          {0xFF,0xFF,0xC1,0xF7,0xF7,0xF7,0xFF,0xFF},/*"cay",0*/
						  {0xFF,0xFF,0x83,0xEF,0xEF,0xEF,0xFF,0xFF},/*"未命名文件",0*/
						  
						  {0xFF,0xFF,0xF7,0xEB,0xDD,0xFF,0xFF,0xFF},/*laugth*/
						  {0xFF,0xFF,0xEF,0xD7,0xBB,0xFF,0xFF,0xFF},/*"未命名文件",0*/

                          {0xEF,0x83,0xEB,0x83,0xAF,0x83,0xEF,0xFF},/*"未命名文件",0*/
						  {0xEF,0x83,0xEB,0x83,0xAF,0x83,0xEF,0xFF},/*"未命名文件",0*/
						  
						  
};
unsigned char rx_buff[MAX_LEN] = {0};
volatile unsigned char rxlen = 0;
unsigned char cmdnum = 0;

void USART_Init( unsigned int ubrr)
{
	cli();
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B |= 0x98;
	/* Set frame format: 8data, 1stop bit */
	UCSR0C |= 0x06;
	sei(); 
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
		UDR0 = data;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) );
	/* Get and return received data from buffer */
	return UDR0;
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR0A & (1<<RXC0) ) 
		dummy = UDR0;
}

void sendstr(unsigned char *s)
{
	while(*s)
	{
		USART_Transmit(*s);
		s++;
	}
}

void time_init(void)
{
	cli();
	TCCR1A=0X00;
	TCNT1H=0XE5;
	TCNT1L=0XEE;
	TCCR1B=0X01;
	TIMSK1 |= 0x01;
	sei();

}

void disp_data_shift(void)
{
	static num = 0;
	int i=0, j=0;
	unsigned char tmpdata[4] = {0};
	
	//data[0][num]<<8|((0x01<<num))
	
	tmpdata[3] = (0x01<<num);
	tmpdata[2] = data[(cmdnum<<1)+1][num];
	tmpdata[1] = (0x01<<num);
	tmpdata[0] = data[(cmdnum<<1)][num];
	
	
	for(i=0; i<4; i++)
	{
		
		
		for(j=0; j<8; j++)
		{
			if(i < 2)
			{
				if ((tmpdata[i]<<j)&0x80)
				{
				//	PORTB |= 1<<PB2;
					PORTD |= 1<<2;
				}
				else
				{
				//	PORTB &= 0<<PB2;
					PORTD &= 0<<2;
				}
				RSH_REG_LOW;
				RSH_REG_HIGH;
			}
			
			else
			{
				if ((tmpdata[i]<<j)&0x80)
				{
					PORTB |= 1<<PB2;
				//	PORTD |= 1<<2;
				}
				else
				{
					PORTB &= 0<<PB2;
				//	PORTD &= 0<<2;
				}	
				LSH_REG_LOW;
				LSH_REG_HIGH;
			}
		
		}
		
	}
		LST_REG_LOW;
		LST_REG_HIGH;
		
		RST_REG_LOW;
		RST_REG_HIGH;
		num++;
	if(num > 7)
		num = 0;
		
}

unsigned char cal_checksum(unsigned char *pdata,unsigned int len)
{
	int i = 0;
	unsigned char checksum = 0;
	for(i = 0; i<len; i++)
	{
		checksum += pdata[i];
	}
	return checksum;
}

void cmd_proc(void)
{
	int i;
	unsigned char checksum = 0;
	if(0x5a == rx_buff[0])
	{
		checksum = cal_checksum(&(rx_buff[1]),2);
		if(checksum == rx_buff[3])
		{
			cmdnum = rx_buff[2];
			sendstr(rx_buff);
		}
		else
		{
			
		}
		
	}
	
}

int main(void)
{

	
	
	unsigned char rec;
	int i;
	
	DDRB |=0x07;
	DDRD |=0x1C;
	PORTB &=0xF8;
	PORTD &=0xE3;
	USART_Init(UBRR);
	time_init();
	
	while(1)
	{
		if(5 == rxlen)
		{
			cmd_proc();
			rxlen = 0;
		}
	}
	return 0;
	
}



ISR(USART_RX_vect)

{
	cli();
	
//	USART_Transmit(UDR0);
	
	if(rxlen < MAX_LEN)
	{
		rx_buff[rxlen] = UDR0;
		rxlen++;
		
	}
	
	else
	{
		rxlen = 0;
	}
	
	sei();
	
	
}


ISR(TIMER1_OVF_vect)

{
	
	cli();
	TCCR1B=0X00;
	TCNT1H=0XE5;
	TCNT1L=0XEE;
	TCCR1B=0X01;
	sei();
	disp_data_shift();

}


