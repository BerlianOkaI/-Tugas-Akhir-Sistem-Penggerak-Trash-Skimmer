/*
 * Sistem Penggerak - Transmitter.cpp
 *
 * Created: 01/12/2023 10:14:09
 * Author : ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "RF433UART.h"
#include "Misc.h"
#include "JoystickButton.h"

unsigned long time_ms;
unsigned char counter;

// Inisiasi Data
RFData myRFData;
JoystickButton myJSButton;

/** SETUP **/
void setup(void)
{
	counter = 1;
	DDRB = 0x01;
	// Initiating UART for RF Transmission
	//myRFData.AT_Setting();
	myRFData.init();
	
	// Initiating ADC and Timer1 for Joystick Data Routine
	myJSButton.init();
	
	sei();
	UART_TxChar(10);
	
}

/** INTERRUPTS  **/
ISR(USART_TX_vect)
{
	myRFData.ISR_transmitting();
}
ISR(USART_RX_vect)
{
	// Empty
}
ISR(TIMER1_OVF_vect)
{
	time_ms += 2;
	myJSButton.read_button();
}
ISR(ADC_vect)
{
	PORTB = (PORTB & (~0x01))|(0x01 & (~PORTB));
	myJSButton.read_joystick();
}

/** MAIN ALGORITHM **/
int main(void)
{
	setup();
    /* Replace with your application code */
	
	UART_TxStr("Hello World!");
	
    while (1) 
    {
		if (time_ms > 40)
		{
			time_ms = 0;
			myRFData.TX_update_data(
				myJSButton.return_x_data(),
				myJSButton.return_y_data(),
				myJSButton.return_button_data()
			);
		}
    }
	return 0;
}

