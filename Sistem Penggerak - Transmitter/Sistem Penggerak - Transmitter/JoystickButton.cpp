/*
 * JoystickButton.cpp
 *
 * Created: 01/12/2023 15:16:56
 *  Author: ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "JoystickButton.h"

char dummy;

void timer1_init(void)
{
	// Set the top value of counter in ICR1 register
	ICR1 = T1_TOP;
	// Set the waveform generation mode (LSB part) and compare output mode to TCCR1A
	TCCR1A = T1_WGMODE_A | T1_COMP_MODE;
	// Set the waveform generation mode (MSB part) to TCCR1B
	TCCR1B = T1_WGMODE_B;
	// Enable Timer1 OVF interrupt
	TIMSK1 = (1 << TOIE1);
	// Start timer1
	TCNT1 = 0;
	TCCR1B = TCCR1B | T1_PRESCALER;
}

void ADC_Init(void)
{
	// Select reference source and adjust settings
	ADMUX = REF_MODE | ADC_ADJUST;
	// ADC Configuration
	ADCSRA = ADC_CONFIG;
	// Select Auto Trigger Source
	ADCSRB = ADAT_SOURCE;
	// Configure the data direction of used adc pins as inputs
	ADC_DDR = ADC_DDR & (~ADC_USED_PINS);
	DIDR0 = ADC_USED_PINS;
	// ADC is ready to be started
}

void ADC_change_mux(unsigned char channel)
{
	ADMUX = (ADMUX & (~MUX_BITS))|(MUX_BITS & channel);
}

unsigned char ADC_read_8bit(void)
{
	return (unsigned char)ADCH;
}

JoystickButton::JoystickButton(void)
{
	// Initiating value of readed adc and button state
	this->_x_data = 128;
	this->_y_data = 128;
	this->_button_state = 0x00;
	this->_pot_state = X_POT;
	
	ADC_change_mux(JOY_X);
}
void JoystickButton::init(void)
{
	// Configure button pins as input
	BUTTON_DDR = BUTTON_DDR & (~BUTTON_DDRCONFIG);
	// Enable/Disable internal pullup resistor
	BUTTON_PORT = BUTTON_PORT & (~BUTTON_DDRCONFIG);		// Disable internal pullup resistor
	timer1_init();
	ADC_Init();
	
	ADCSRA = ADCSRA | (1 << ADSC);
}

void JoystickButton::data_reset(void)
{
	this->_x_data = 128;
	this->_y_data = 128;
	this->_button_state = 0x00;
}

void JoystickButton::read_joystick(void)
{
	switch (this->_pot_state)
	{
	case X_POT:
		// Read Converted analog data in A/D Register
		dummy = (char)ADCL;
		this->_x_data = (unsigned char)ADCH;
		// Change mux for next conversion
		this->_pot_state = Y_POT;
		ADC_change_mux(JOY_Y);
		break;
	case Y_POT:
		// Read Converted analog data in A/D Register
		dummy = (char)ADCL;
		this->_y_data = (unsigned char)ADCH;
		// Change mux for next conversion
		this->_pot_state = X_POT;
		ADC_change_mux(JOY_X);
		break;
	}
}

void JoystickButton::read_button(void)
{
	this->_button_state = BUTTON_PIN;
	this->_button_state = (~(this->_button_state)) & BUTTON_DDRCONFIG;
}

unsigned char JoystickButton::return_x_data(void)
{
	return this->_x_data;
}

unsigned char JoystickButton::return_y_data(void)
{
	return this->_y_data;
}

unsigned char JoystickButton::return_button_data(void)
{
	return this->_button_state;
}