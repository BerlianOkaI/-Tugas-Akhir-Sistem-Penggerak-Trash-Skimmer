/*
 * Sistem Penggerak - Receiver.cpp
 *
 * Created: 01/12/2023 09:49:07
 * Author : ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "RF433UART.h"
#include "Conveyor.h"
#include "ESC_Control.h"
#include "TimerConfig.h"

#define LED_BUILTIN			(1 << PORTB5)
#define LED_BUILTIN_PORT	PORTB
#define LED_BUILTIN_DDR		DDRB

#define ACC_BUTTON			(1 << PORTD4)					// Pin for Accelerating button
#define CONVIN_BUTTON		(1 << PORTD5)					// Pin for Conveyor in button
#define CONVOUT_BUTTON		(1 << PORTD3)					// Pin for Conveyor out button
#define TIMEOUT_LIMIT		1000000							// Limit of timeout in us

typedef unsigned char u_char;

class MainControl
{
private:
	unsigned long _timeout_us;
	u_char _analog_joystick[2];
	u_char _button_status;
	RFData* _RFData_pointer;
	ESC_Controller* _ESC_Controller_pointer;
public:
	MainControl(RFData* RFData_address, ESC_Controller* ESC_Controller_address)
	{
		this->_RFData_pointer = RFData_address;
		this->_ESC_Controller_pointer = ESC_Controller_address;
		this->_analog_joystick[0] = 128;
		this->_analog_joystick[1] = 128;
		this->_button_status = 0x00;
	}
	void all_init(void)
	{
		this->_timeout_us = 0;
		conveyorSetup();
		this->_RFData_pointer->init();
		this->_ESC_Controller_pointer->init();
	}
	void ISR_Timer_Routine(unsigned long deltime)
	{
		// Update the timeout timer
		this->_timeout_us += deltime;
		
		if (this->_timeout_us > TIMEOUT_LIMIT)
		{
			this->_timeout_us = 0;
			// Force the RFData RX Buffer Data Train to be default!
			this->_RFData_pointer->RX_default_data();
		}
		
	}
	
	void update_device_state(void)
	{
		if (this->_RFData_pointer->RX_return_data(this->_analog_joystick, &this->_button_status))
		{
			this->_timeout_us = 0;
			// Received new data
			// ESC Controller update
			// ...
			
			// Change conveyor action
			switch (this->_button_status & (CONVIN_BUTTON | CONVOUT_BUTTON))
			{
			case CONVIN_BUTTON:
				conveyorWrite(1, 190);
				break;
			case CONVOUT_BUTTON:
				conveyorWrite(2, 190);
				break;
			default:
				conveyorWrite(0, 190);
				break;
			}
			
			if (this->_button_status & ACC_BUTTON)
			{
				if (this->_analog_joystick[0] < 64)
				{
					// Turn left
					this->_ESC_Controller_pointer->WritePercentage(0, 10);	
				}
				else if (this->_analog_joystick[0] > 192)
				{
					// Turn right
					this->_ESC_Controller_pointer->WritePercentage(6, 0);
				}
				else
				{
					// Move forward, until our enemies are fallen
					this->_ESC_Controller_pointer->WritePercentage(6, 10);
				}
			}
			else
			{
				this->_ESC_Controller_pointer->WritePercentage(0, 0);
			}
		}	
	}
};

RFData myRFData;
ESC_Controller myPropellers;
MainControl myControl(&myRFData, &myPropellers);

// SETUPS
void setup(void)
{	
	// Set the LED Pin as Output
	LED_BUILTIN_DDR = LED_BUILTIN_DDR | LED_BUILTIN;
	// Initiating ESC (Propellers), conveyor, and RF433 Communication through MainControl Object
	myControl.all_init();
	// Enabling global interrupt
	sei();
}

// INTERRUPTS
ISR(TIMER2_OVF_vect)
{
	/* 
	Overflown Flag Interrupt of Timer 2.
	Please, refer to Conveyor.cpp to configure the timer 2
	*/
	myControl.ISR_Timer_Routine(16384);
	
}
ISR(USART_RX_vect)
{
	/*
	Receive complete interrupt. RFData.ISR_receiving()
	must be executed in this interrupt.
	*/
	myRFData.ISR_receiving();
}

ISR(USART_TX_vect)
{
	// Do nothing. Just clearing TXC Flag
}

int main(void)
{
	setup();
	// First, let's try to run the conveyor
	conveyorWrite(0, 190);
	LED_BUILTIN_PORT = LED_BUILTIN_PORT | LED_BUILTIN;
	delay1ms(6000);
	
    while (1) 
    {
		myControl.update_device_state();
		delay1ms(100);
    }
}
