/*
 * Lab 2.c
 *
 * Created: 9/24/2017 12:51:01 AM
 * Author : Myles Cai & Julian Mickelson
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "uart.h"

#define set(reg,bit)		reg |= (1<<(bit))
#define clear(reg,bit)		reg &= ~(1<<(bit))
#define toggle(reg,bit)		reg ^= (1<<(bit))
#define check(reg,bit)		(bool)(reg & (1<<(bit)))

#define clockdivide(val)	CLKPR = (1<<CLKPCE); CLKPR=val

#define TRIG 2 // output compare
#define ECHO 0 // input capture

volatile int risetime;
volatile int falltime;
volatile int captFlag;
volatile int ADCFlag;

void adcsetup(void);
void timer0setup(void);
void pulse(void);
//void DAC(int);

int main(void)
{
	sei(); // enable interrupts
	clockdivide(0);
	uart_init(); // serial comm
	
	adcsetup();
	timer0setup();

	clear(DDRB,ECHO); // clear for input capture
	set(DDRB,TRIG);   // set output for pulse, timer1
	
	set(PORTB,TRIG); // set output pulse to high
	
	pulse();
	
	set(TCCR1B,ICES1); // input capture ICP1 rising edge, ICP1 multiplexed on B1
	// this will set ICF1 flag
	
	set(TIMSK1,ICIE1); // timer1 input capture interrupt enable
	// corresponding interrupt vector is executed when the ICF1 flag in TIFR1 is set

    while (1) 
    {
		if (captFlag) {
			printf("%s","risetime: ");
			printf("%u", risetime);
			printf("\n");
					
			printf("%s", "falltime: ");
			printf("%u", falltime);
			printf("\n");

			printf("%s","difference: ");
			printf("%u", falltime-risetime);
			printf("\n");
			
			captFlag = 0;
		}
		
		if (ADCFlag == 1000) {
			printf("%s","Light Sensor: ");
			printf("%u",ADC);
			printf("\n");
			
			ADCFlag = 0;
		}
    }
}

void pulse(void) {
		
	clear(TCCR1B,CS12);
	clear(TCCR1B,CS11);
	set(TCCR1B,CS10);	// Timer 1 prescaler /1
		
	set(TCCR1B,WGM13);
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10); // up to OCR1A (Fast PWM Mode 15)

	set(TCCR1A,COM1B1);
	clear(TCCR1A,COM1B0); // toggle OC1B (B2) on compare match

	OCR1A = 65536; // 160 ticks for 10 us at 16MHz
	OCR1B = 160;
}


void adcsetup (void) {

	clear(ADMUX,REFS1);
	set(ADMUX,REFS0);	// use AVcc as reference voltage; use ext capacitor

	set(ADCSRA,ADPS2);
	set(ADCSRA,ADPS1);
	set(ADCSRA,ADPS0);  // prescaler /128 = 125kHz

	clear(ADMUX,MUX3);
	clear(ADMUX,MUX2);
	clear(ADMUX,MUX1);
	clear(ADMUX,MUX0);  // channel selection, ADC0
	
	set(ADCSRA,ADATE);  // en/dis-able auto trigger
	set(ADCSRA,ADIE);   // enable adc interrupt
	clear(DIDR0,ADC0D); // disable digital input for ADC0

	set(ADCSRA,ADEN);   // ADC system enable
	set(ADCSRA,ADSC);   // start conversion // will need to set every time if not free running
}

void timer0setup(void){

	set(DDRB,7); // set up output pin for timer0

	clear(TCCR0B,WGM02);
	set(TCCR0A,WGM01);
	clear(TCCR0A,WGM00); // mode 2, CTC @ OCRA [pg106]

	clear(TCCR0A,COM0A1);
	set(TCCR0A,COM0A0); // toggle OC0A on compare match [pg104]

	OCR0A = 125;

	clear(TCCR0B,CS02);
	set(TCCR0B,CS01);
	set(TCCR0B,CS00); // prescaler /64 = 250kHz

	/* this enables a range that can give us 1 - 2kHz
	1000 Hz -> OCR0A = 125 (since we're toggling), otherwise it'd be 250
	1122 Hz -> OCR0A = 111
	1260 Hz -> OCR0A = 99
	1335 Hz -> OCR0A = 94
	1498 Hz -> OCR0A = 83
	1682 Hz -> OCR0A = 74
	1888 Hz -> OCR0A = 66
	2000 Hz -> OCR0A = 62

	write an if statement that changes OCR0A depending on ADC value
	could change it in the ADIF interrupt

	*/
}

ISR(TIMER1_CAPT_vect){
	// ICF1 will be cleared automatically
	if(check(TCCR1B,ICES1)){
		risetime = ICR1;
		clear(TCCR1B,ICES1); // input capture ICP1 falling edge
	} else {
		falltime = ICR1;
		
		captFlag = 1;
	
		set(TCCR1B,ICES1); // input capture ICP1 rising edge
	}

}

ISR(ADC_vect){
	// ADIF will be cleared automatically
	// interrupt when ADC conversion complete

	/* this enables a range that can give us 1 - 2kHz
	1000 Hz -> OCR0A = 125 (since we're toggling), otherwise it'd be 250
	1122 Hz -> OCR0A = 111
	1260 Hz -> OCR0A = 99
	1335 Hz -> OCR0A = 94
	1498 Hz -> OCR0A = 83
	1682 Hz -> OCR0A = 74
	1888 Hz -> OCR0A = 66
	2000 Hz -> OCR0A = 62

	write an if statement that changes OCR0A depending on ADC value
	could change it in the ADIF interrupt

	*/

	if(ADC < 128){
		OCR0A = 125;
	} else if (ADC < 128*2) {
		OCR0A = 111;
	} else if (ADC < 128*3) {
		OCR0A = 99;
	} else if (ADC < 128*4) {
		OCR0A = 94;
	} else if (ADC < 128*5) {
		OCR0A = 83;
	} else if (ADC < 128*6) {
		OCR0A = 74;
	} else if (ADC < 128*7) {
		OCR0A = 66;
	} else {
		OCR0A = 62;
	}
	
	ADCFlag += 1;
}

//void DAC(int x){
	//
	//set(DDRB,2);
	//set(DDRB,3);
	//set(DDRB,4); // output group number on PB2-4
	//
	//switch(x) {
//
		//case 0:
		//clear(PORTB,2);
		//clear(PORTB,3);
		//clear(PORTB,4);
		//break;
	//
		//case 1:
		//clear(PORTB,2);
		//clear(PORTB,3);
		//set(PORTB,4);
		//break;
		//
		//case 2:
		//clear(PORTB,2);
		//set(PORTB,3);
		//clear(PORTB,4);
		//break;
		//
		//case 3:
		//clear(PORTB,2);
		//set(PORTB,3);
		//set(PORTB,4);
		//break;
		//
		//case 4:
		//set(PORTB,2);
		//clear(PORTB,3);
		//clear(PORTB,4);
		//break;
		//
		//case 5:
		//set(PORTB,2);
		//clear(PORTB,3);
		//set(PORTB,4);
		//break;
		//
		//case 6:
		//set(PORTB,2);
		//set(PORTB,3);
		//clear(PORTB,4);
		//break;
		//
		//case 7:
		//set(PORTB,2);
		//set(PORTB,3);
		//set(PORTB,4);
		//break;					
//
		//default:
		//printf("%s", "error with grouping of ultrasonic");
//
	//}
//}

