/*
 * Conveyor.cpp
 *
 * Created: 29/11/2023 13:30:15
 *  Author: ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Conveyor.h"

/* FUNCTIONS AND PROCEDURES */

void timer2SetUp(void)				// #1
{
	// Clearing TCCR2B register
	TCCR2B = 0x00;
	// Initiating value of timer register and output compare register
	TCNT2 = 0x00;
	OCR2B = 128;
	// Set Waveform of timer 2 as fast PWM
	TCCR2A = (1 << WGM21)|(1 << WGM20);
	// Start timer 2 as 1024 as prescaler. Its running with periodicity of 64 us
	TCCR2B = (1 << CS22)|(1 << CS21)|(1 << CS20);
	// Enable timer 2 overflown interrupt
	TIMSK2 = (1 << TOIE2);
}

void conveyorSetup(void)			// #2
{
	// Setting timer2 configuration
	timer2SetUp();
	// Setting pinup of controlling pin for conveyor
	CONV_DDR = CONV_DDR | (1 << CONV_IN1) | (1 << CONV_IN2) | (1 << CONV_EN);
	// Set CONV_EN as PWM Output (non-inverting mode)
	TCCR2A = (TCCR2A & (~0x30))|(1 << COM2B1);
}

void conveyorWrite(unsigned char dir, unsigned int pulse_length)			// #3
{
	switch(dir)
	{
		case 0:
			// Conveyor doesn't roll
			CONV_PORT = CONV_PORT & (~(1 << CONV_IN1)) & (~(1 << CONV_IN2));
			break;
		case 1:
			// Conveyor roll up
			CONV_PORT = CONV_PORT | (1 << CONV_IN1);
			CONV_PORT = CONV_PORT & (~(1 << CONV_IN2));
			break;
		case 2:
			// Conveyor roll down
			CONV_PORT = CONV_PORT & (~(1 << CONV_IN1));
			CONV_PORT = CONV_PORT | (1 << CONV_IN2);
			break;
	}
	OCR2B = pulse_length;
}




