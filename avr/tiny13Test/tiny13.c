#define F_CPU 1200000UL  // 1.2 MHz

#include <avr/io.h>
//#include <avr/iotn13.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay_basic.h>




const uint8_t __attribute__ ((progmem)) sinArray[64] =
{
	0,   0,   2,   5,   9,  15,  21,  29,
  37,  46,  56,  67,  79,  90, 103, 115,
 128, 140, 152, 165, 176, 188, 199, 209,
 218, 226, 234, 240, 246, 250, 253, 255,
 255, 255, 253, 250, 246, 240, 234, 226,
 218, 209, 199, 188, 176, 165, 152, 140,
 128, 115, 103,  90,  79,  67,  56,  46,
  37,  29,  21,  15,   9,   5,   2,   0,
};


volatile uint8_t looper = 1;


ISR(INT0_vect)
{
   cli();
   _delay_ms(50);
   if(!(PINB & 2))
   {
		looper = 0;
		while(!(PINB & 2));
   }

   //clear any new interrupt flags
   GIFR |= 1 << INTF0;

   sei();
}

//prototype
uint8_t myrand(uint8_t modVal);

int main(void)
{
   //MCUCR = (1 << ISC01);
   GIMSK  |= (1 << INT0);

	//Disable the ADC to save power
	ACSR |= (1 << ACD);

	//configure port B pins 0,2,3 and 4 as ouputs
	DDRB = (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4);

	//enable internal pullup on port b pin 1 for the button
	PORTB |= (1 << 1);

	//NOTE: if we run outta space, I'll comment this out
	sei();

	while(1)
	{
		/**
		 * Sleep Mode, default mode
		 */
		{
			cli();
			MCUCR |= (1 << SE) | (1 << SM1);
			//re-enable interrupts
			sei();
			sleep_cpu();
			//disable sleem mode
			MCUCR &= ~(1 << SE);
		}
		looper = 1;


		/**
		 * Pulse or "breath" mode.
		 */
		uint8_t dutyInc = 0;
		uint8_t dutySlowDown = 20;
		uint8_t counter = 0;
		while(looper)
		{
			counter ++;
			if(counter >= dutySlowDown)
			{
				dutyInc ++;
				counter = 0;
			}
			uint8_t dutyCycle = pgm_read_byte(&sinArray[dutyInc]);
			for(int i = 0; i < 256; i ++)
			{
				if(i < dutyCycle) PORTB |= (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4);
				else PORTB &= ~((1 << 0) | (1 << 2) | (1 << 3) | (1 << 4));
			}
			if(dutyInc >= 63) dutyInc = 0;
		}
		looper = 1;


		/**
		 * Pseudo candle mode.
		 */
		counter = 0;
		uint8_t rand = 0;
		uint8_t dutyCycle = 0;
		while(looper)
		{
			counter ++;
			if(counter >= rand)
			{
				rand = myrand(64);
				counter = 0;
				dutyCycle = pgm_read_byte(&sinArray[myrand(20) + 10]);
			}
			for(int i = 0; i < 256; i ++)
			{
				if(i < dutyCycle) PORTB |= (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4);
				else PORTB &= ~((1 << 0) | (1 << 2) | (1 << 3) | (1 << 4));
			}

		}
		looper = 1;


		/**
		 * Test mode.
		 */
		while(looper)
		{
			PORTB |= (1 << 0);
			_delay_ms(500);
			PORTB &= ~(1 << 0);
			PORTB |= (1 << 2);
			_delay_ms(500);
			PORTB &= ~(1 << 2);
			PORTB |= (1 << 3);
			_delay_ms(500);
			PORTB &= ~(1 << 3);
			PORTB |= (1 << 4);
			_delay_ms(500);
			PORTB &= ~(1 << 4);
		}
		looper = 1;




	
	}




	return 0;
}


uint8_t myrand(uint8_t modVal)
{
	static uint32_t seed = 1;

	seed = ((seed * 16807) + 100) % 2147483647;
	return seed % modVal;
}
