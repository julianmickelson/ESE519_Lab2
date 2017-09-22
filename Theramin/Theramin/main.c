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

#define set(reg,bit)		reg |= (1<<(bit))
#define clear(reg,bit)		reg &= ~(1<<(bit))
#define toggle(reg,bit)		reg ^= (1<<(bit))
#define check(reg,bit)		(bool)(reg & (1<<(bit)))


int main(void)
{
	sei();
	
	set(DDRB,5); // output b5
	set(DDRB,1);
	
	clear(TCCR1B,CS12); // clock prescaler /1
	clear(TCCR1B,CS11);
	set(TCCR1B,CS10);
	
	OCR1A = 18181;
	
	set(TCCR1B,WGM13);
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10); // up to OCR1A, mode 15
	
	set(TCCR1B,ICES1); // input capture ICP1 rising edge
	// this will set ICF1
	
	set(TIMSK1,ICIE1); // timer1 input capture interrupt enable
	// corresponding interrupt vector is executed when the ICF1 flag in TIFR1 is set
	set(TIMSK1,OCIE1A); // enable TIMER1_COMPA
	
    while (1) 
    {
    }
}

ISR(TIMER1_COMPA_vect){
	toggle(PORTB,1);
}

ISR(TIMER1_CAPT_vect){
	// ICF1 will be cleared automatically
	toggle(PORTB,5); // 
	toggle(TCCR1B,ICES1); 
}

