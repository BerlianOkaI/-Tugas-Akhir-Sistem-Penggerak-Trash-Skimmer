/*
 * ESC_Control.cpp
 *
 * Created: 29/11/2023 14:49:37
 *  Author: Berlian Oka Irvianto (Indonesia)
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ESC_Control.h"
#include "TimerConfig.h"

void timer1SetUp(void)																// #1
{
	/*
	Setting Timer1 agar dapat berjalan dengan periode sebesar 20 ms.
	Nilai TOP dari Timer1 Register TCNT1 diset sebagai nilai ICR1 sehingga
	WGM13:0 mesti diset sedemikian rupa sehingga TOP dari TCNT1 adalah ICR1.
	
	Periode yang dihasilkan adalah 20 ms dengan frekuensi clock sebesar 16 MHz.
	Dengan menggunakan prescaler sebesar 256, maka diperolehlah nilai TOP value
	yang mesti dimiliki TCNT1 sebesar 1250. Untuk bisa mendapatkan mode ini, maka
	WGM13:0 = 0b1110.
	*/
	// Initiating and clearing TCCR1 register
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	// Set-up Waveform Generation Mode as Fast PWM (with TOP value would be ICR1)
	TCCR1A = TCCR1A | (1 << WGM11);
	TCCR1B = TCCR1B | (1 << WGM13) | (1 << WGM12);
	// Set-up Output Compare Mode as Non-Inverting Fast PWM
	TCCR1A = TCCR1A | (1 << COM1A1) | (1 << COM1B1);
	// Set the value of ICR1 to define new TOP of TCNT1 Register
	ICR1 = ESCPWM_TOP;
	// Initiating value of TNCT1 and OCR1x
	TCNT1 = 0x00;
	OCR1A = ESC_ZPULSE;
	OCR1B = ESC_ZPULSE;
	// Start the timer1 with 8 as prescaler (CS12:0 = 0b100)
	TCCR1B = TCCR1B | ESCPWM_PRESCALER;
}
void ESCSetup(void)																	// #2
{
	/** 
	This code provide a basic setup for both ESC
	in Trash Skimmer. This include setting up PWM generator
	through Timer1 and setting up pin direction of 
	ESC pin as output. This code doesn't provide
	calibration process of ESC.	
	**/
	// Setting the ESC Control Pin as output
	ESC_DDR = ESC_DDR | (1 << ESC_IN1) | (1 << ESC_IN2);
	// Setting the timer1 configuration
	timer1SetUp();
}
void ESC_Controller::init(void)
{
	// ESC Initial pulse width in bit
	this->_motor_L = ESC_ZPULSE;
	this->_motor_R = ESC_ZPULSE;
	// ESC Initial power level in percentage
	this->_motor_pow_level[ESC_LEFT] = 0.0;
	this->_motor_pow_level[ESC_RIGHT] = 0.0;
	
	ESCSetup();
	this->Write(this->_motor_L, this->_motor_R);
	//this->Throttling(); 
}

void ESC_Controller::Throttling(void)
{
	/* Calibrating range of throttle for ESC */
	// Set the input signal with maximum pulse (2ms pulse) and wait for ESC to start beeping (wait for ~5 sec.)
	this->Write(ESC_MAXPULSE, ESC_MAXPULSE);
	delay1ms(5000);
	// Set the input signal with minimum pulse (1ms pulse) and wait for ESC to start beeping (wait for ~3 sec.)
	this->Write(ESC_MINPULSE, ESC_MINPULSE);
	delay1ms(3000);
	// The ESC should be calibrated from this point I supposed
}

void ESC_Controller::Write(unsigned int ESC1_PulseWidth, unsigned int ESC2_PulseWidth)			// #3
{
	/*
	Give ESC PWM Signal that control each of ESC Control Pin with
	a given velocity that is proportional (?) to pulse_width assigned
	to them
	*/
	// This is for ESC1 Control Pin
	if (ESC1_PulseWidth >= ESC_MAXPULSE)		{ESC_PWMREG1 = ESC_MAXPULSE;}
	else if(ESC1_PulseWidth <= ESC_MINPULSE)	{ESC_PWMREG1 = ESC_MINPULSE;}
	else										{ESC_PWMREG1 = ESC1_PulseWidth;}
	
	// This is for ESC2 Control Pin
	if (ESC2_PulseWidth >= ESC_MAXPULSE)		{ESC_PWMREG2 = ESC_MAXPULSE;}
	else if(ESC2_PulseWidth <= ESC_MINPULSE)	{ESC_PWMREG2 = ESC_MINPULSE;}
	else										{ESC_PWMREG2 = ESC2_PulseWidth;}
}
void ESC_Controller::WritePercentage(float ESC1_percentage, float ESC2_percentage)
{
	/*
	Give output to ESC in percentage (0.0% means giving minimum pulse width input to ESC (1 ms pulse)
	and 100.0% means giving maximum pulse width input to ESC (2 ms pulse))
	*/
	float grad, intercept;
	int value1, value2;
	grad		= ((float)ESC_MAXPULSE - (float)ESC_ZPULSE)/(100.0 - 0.0);
	intercept	= (((float)ESC_ZPULSE * 100.0) - ((float)ESC_MAXPULSE * 0.0))/(100.0 - 0.0);
	
	value1 = (int)(grad * ESC1_percentage) + (int)intercept;
	value2 = (int)(grad * ESC2_percentage) + (int)intercept;
	
	this->Write(value1, value2);
}

void ESC_Controller::Clear(void)																	// #4
{
	ESC_PWMREG1 = ESC_ZPULSE;
	ESC_PWMREG2 = ESC_ZPULSE;
}