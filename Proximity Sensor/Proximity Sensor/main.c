/* ESE 519 LAB 2
	Myles Cai
	Julian Mickelson
*/

#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include "uart.h"

#define set(reg,bit)		reg |= (1<<(bit))
#define clear(reg,bit)		reg &= ~(1<<(bit))
#define toggle(reg,bit)		reg ^= (1<<(bit))
#define check(reg,bit)		(bool)(reg & (1<<(bit)))

volatile int riseTime; 
volatile int fallTime; 
volatile int pulseTime; 
volatile int flag;



int main(void)
{
	uart_init();
	sei();	// enable interrupts
	
	clear(DDRB,0); // b0 input capture
	set(DDRB,1); // b1 output compare
	
	set(PORTB,1); // set b1 high
	
	clear(TCCR1B,CS12); // no clock prescaler 
	clear(TCCR1B,CS11);
	clear(TCCR1B,CS10);
	
	OCR1A = 160;	// 10us at prescale 0 
	
	clear(TCCR1B,WGM13);  // CTC OCR1A top, mode 4
	set(TCCR1B,WGM12);
	clear(TCCR1A,WGM11);
	clear(TCCR1A,WGM10); 
	
	clear(TCCR1A,COM1A1); // toggling OC1A (PB1) on OCR1A match
	set(TCCR1A,COM1A0); 
	
	set(TCCR1B,ICES1); // input capture ICP1 rising edge
	
    while (1) 
    {
		if (flag) {
			printf("%u\n", pulseTime);
			flag = 0;
		}
    }
}

ISR(TIMER1_COMPA_vect){
	TCNT1 = 0; // start clock from 0
	set(TIMSK1,ICIE1); // timer1 input capture interrupt enable
}

ISR(TIMER1_CAPT_vect){
	riseTime = ICR1;
	clear(TCCR1B,ICES1); // input capture ICP1 falling edge
	fallTime = ICR1;
	
	pulseTime = fallTime - riseTime;
	flag = 1;
}


