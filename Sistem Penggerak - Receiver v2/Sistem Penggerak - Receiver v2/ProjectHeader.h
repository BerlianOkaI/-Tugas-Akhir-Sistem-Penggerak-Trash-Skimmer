/*
 * ProjectHeader.h
 *
 * Created: 25/05/2024 10:23:02
 *  Author: ASUS
 */ 


#ifndef PROJECTHEADER_H_
#define PROJECTHEADER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "TypeDef.h"
#include "RF433UART.h"
#include "Conveyor.h"
#include "ESC_Control.h"
#include "TimerConfig.h"
#include "FuzzyLogic.h"

#define LED_BUILTIN			(1 << PORTB0)
#define LED_BUILTIN_PORT	PORTB
#define LED_BUILTIN_DDR		DDRB

#define ACC_BUTTON			(1 << PORTD4)					// Pin for Accelerating button
#define CONVIN_BUTTON		(1 << PORTD5)					// Pin for Conveyor in button
#define CONVOUT_BUTTON		(1 << PORTD3)					// Pin for Conveyor out button
#define VARCONV_BUTTON		(1 << PORTD2)					// Variable Conveyor speed
#define TIMEOUT_LIMIT		1000000L						// Limit of timeout in us
#define TIMEOUT_LED			3000000L						// Period of LED blinking

typedef unsigned char u_char;

class MainControl
{
	private:
	unsigned long _timeout_us;					// Time-out for refreshing trash skimmer state
	u_char _analog_joystick[2];					// Data buffer (8-bit) for joystick value (X and Y axis)
	u_char _button_status;						// Pressed button data from transmitter
	RFData* _RFData_pointer;					// pointer of RF Communication control
	ESC_Controller* _ESC_Controller_pointer;	// pointer of ESC Controller
	public:
	MainControl(RFData* RFData_address, ESC_Controller* ESC_Controller_address);
	void all_init(void);
	void ISR_Timer_Routine(unsigned long deltime);
	void update_device_state(void);
};

#endif /* PROJECTHEADER_H_ */