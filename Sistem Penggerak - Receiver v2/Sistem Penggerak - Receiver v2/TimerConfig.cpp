/*
 * TimerConfig.cpp
 *
 * Created: 31/10/2023 16:44:55
 *  Author: ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TimerConfig.h"
#include "TypeDef.h"

/* FUNCTIONS AND PROCEDURES */
void delay1ms(unsigned int length)
{
	// Creating delay of 1 ms times length
	// Using Fosc = 16 (MHz)
	// Using Timer2 as clock source
	unsigned int _iter;
	for (_iter=0; _iter<length; _iter++){
		TCNT2 = 0x06;
		TCCR2B = _b(CS22);	// Start Timer using 64 as clk divider. Thus, we have period of 4us
		while(!(TIFR2 & _b(TOV2)));	// Polling TimerOverflown0 Flag Until It is Set
		TCCR2B = 0x00;					// Stop timer0
		TIFR2 = TIFR2 | _b(TOV2);		// Put 1 to TOV2 bit to clear the interrupt flag
	}
}

