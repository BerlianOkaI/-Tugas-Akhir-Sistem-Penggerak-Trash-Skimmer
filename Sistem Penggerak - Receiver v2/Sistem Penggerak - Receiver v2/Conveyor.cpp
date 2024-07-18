/*
 * Conveyor.cpp
 *
 * Created: 29/11/2023 13:30:15
 *  Author: ASUS
 */ 

#include "Conveyor.h"

/* FUNCTIONS AND PROCEDURES */

void timer0SetUp(void)				// #1
{
	// Clearing TCCR0B register
	TCCR0B = 0x00;
	// Initiating value of timer register and output compare register
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;
	// Set Waveform of timer 0 as Phase Correct PWM for both Output Compare
	TCCR0A = _b(COM0A1) | _b(COM0B1) | _b(WGM00);
	// Start timer 0 with prescaler of 64 so that we have PWM frequency of 490.2 Hz
	TCCR0B = _b(CS02) | _b(CS00);
	
	// Enable timer 0 overflown interrupt for every #2040 us
	TIMSK0 = _b(TOIE0);
}

void conveyorSetup(void)			// #2
{
	// Setting timer0 configuration
	timer0SetUp();
	// Setting data direction of I/O pin of controlling pin for conveyor
	CONV_DDR = _b(CONV_IN1) | _b(CONV_IN2) | _b(CONV_EN)
				| _b(CONV2_IN1) | _b(CONV2_IN2) | _b(CONV2_EN);
	// Re-initiate digital output of control I/O pin
	CONV_PORT = CONV_PORT & (~_b(CONV_IN1)) & (~_b(CONV_IN2)) & (~_b(CONV_EN))
				& (~_b(CONV2_IN1)) & (~_b(CONV2_IN2)) & (~_b(CONV2_EN));
}

void conveyorWrite(unsigned char dir, unsigned int pulse_length)			// #3
{
	switch(dir)
	{
		case 0:
			// Conveyor doesn't roll
			CONV_PORT = CONV_PORT & (~_b(CONV_IN1)) & (~_b(CONV_IN2));
			break;
		case 1:
			// Conveyor roll up
			CONV_PORT = CONV_PORT | _b(CONV_IN1);
			CONV_PORT = CONV_PORT & (~_b(CONV_IN2));
			break;
		case 2:
			// Conveyor roll down
			CONV_PORT = CONV_PORT & (~_b(CONV_IN1));
			CONV_PORT = CONV_PORT | _b(CONV_IN2);
			break;
		default:
			// No action
			break;
	}
	OCR0A = pulse_length;
}

void conveyorWrite2(unsigned char dir, unsigned int pulse_length)			// #3
{
	switch(dir)
	{
		case 0:
			// Conveyor doesn't roll
			CONV_PORT = CONV_PORT & (~_b(CONV2_IN1)) & (~_b(CONV2_IN2));
			break;
		case 1:
			// Conveyor roll up
			CONV_PORT = CONV_PORT | _b(CONV2_IN1);
			CONV_PORT = CONV_PORT & (~_b(CONV2_IN2));
			break;
		case 2:
			// Conveyor roll down
			CONV_PORT = CONV_PORT & (~_b(CONV2_IN1));
			CONV_PORT = CONV_PORT | _b(CONV2_IN2);
			break;
		default:
			// No action
			break;
	}
	OCR0B = pulse_length;
}


