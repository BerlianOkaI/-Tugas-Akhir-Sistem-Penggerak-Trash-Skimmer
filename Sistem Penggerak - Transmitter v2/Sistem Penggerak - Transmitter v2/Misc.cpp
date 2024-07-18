/*
 * Misc.cpp
 *
 * Created: 01/01/2024 11:53:54
 *  Author: ASUS
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Misc.h"


void delay1ms(unsigned int length)
{
	// Creating delay of 1 ms times length
	// Using Fosc = 16 (MHz)
	// Using Timer0 as clock source
	unsigned int _iter;
	for (_iter=0; _iter<length; _iter++){
		TCNT0 = 0x06;
		TCCR0B = (1 << CS01) | (1 << CS00);	// Start Timer using 64 as clk divider. Thus, we have period of 4us
		while((TIFR0 & (1 << TOV0)) == 0);	// Polling TimerOverflown0 Flag Until It is Set
		TCCR0B = 0x00;						// Stop timer0
		TIFR0 = TIFR0 | (1 << TOV0);		// Put 1 to TOV0 to clear the flag
	}
}
